/*
 * drivers/usb/gadget/f_cdrom.c
 *
 * Function Driver for USB CDROM
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *
 * Based heavily on the file_storage gadget driver in
 * drivers/usb/gadget/file_storage.c and licensed under the same terms:
 *
 * Copyright (C) 2003-2007 Alan Stern
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the above-listed copyright holders may not be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation, either version 2 of that License or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/***********************************************************************/
/* Modified by                                                         */
/* (C) NEC CASIO Mobile Communications, Ltd. 2011                      */
/***********************************************************************/

/* #define DEBUG */
/* #define VERBOSE_DEBUG */
/* #define DUMP_MSGS */


#include <linux/blkdev.h>
#include <linux/completion.h>
#include <linux/dcache.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/kref.h>
#include <linux/kthread.h>
#include <linux/limits.h>
#include <linux/rwsem.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/switch.h>
#include <linux/freezer.h>
#include <linux/utsname.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>

#include <linux/usb.h>
#include <linux/usb_usual.h>
#include <linux/usb/ch9.h>
#include <linux/usb/android_composite.h>

#include "f_cdrom.h"
#include "gadget_chips.h"
#include "comm_usb.h"

#include "f_timer.h"

#define BULK_BUFFER_SIZE           8192

/*-------------------------------------------------------------------------*/

#define DRIVER_NAME        "usb_cdrom"
#define MAX_LUNS           8

static const char shortname[] = DRIVER_NAME;

#define LDBG(lun_cdrom, fmt, args...) \
	do { } while (0)
#define MDBG(fmt,args...) \
	do { } while (0)
#undef VERBOSE_DEBUG
#undef DUMP_MSGS

#define VLDBG(lun_cdrom, fmt, args...) \
	do { } while (0)

#define LERROR(lun_cdrom, fmt, args...) \
	dev_err(&(lun_cdrom)->dev , fmt , ## args)
#define LWARN(lun_cdrom, fmt, args...) \
	dev_warn(&(lun_cdrom)->dev , fmt , ## args)
#define LINFO(lun_cdrom, fmt, args...) \
	dev_info(&(lun_cdrom)->dev , fmt , ## args)

#define MINFO(fmt,args...) \
	printk(KERN_INFO DRIVER_NAME ": " fmt , ## args)

#undef DBG
#undef VDBG
#undef ERROR
#undef WARNING
#undef INFO
#define DBG(d, fmt, args...) \
	dev_dbg(&(d)->cdev->gadget->dev , fmt , ## args)
#define VDBG(d, fmt, args...) \
	dev_vdbg(&(d)->cdev->gadget->dev , fmt , ## args)
#define ERROR(d, fmt, args...) \
	dev_err(&(d)->cdev->gadget->dev , fmt , ## args)
#define WARNING(d, fmt, args...) \
	dev_warn(&(d)->cdev->gadget->dev , fmt , ## args)
#define INFO(d, fmt, args...) \
	dev_info(&(d)->cdev->gadget->dev , fmt , ## args)


/*-------------------------------------------------------------------------*/

/* Bulk-only data structures */

/* Command Block Wrapper */
struct bulk_cb_wrap_cdrom {
    __le32      Signature;            /* Contains 'USBC' */
    u32         Tag;                  /* Unique per command id */
    __le32      DataTransferLength;   /* Size of the data */
    u8          Flags;                /* Direction in bit 7 */
    u8          Lun;                  /* LUN (normally 0) */
    u8          Length;               /* Of the CDB, <= MAX_COMMAND_SIZE */
    u8          CDB[16];              /* Command Data Block */
};

#define USB_BULK_CB_WRAP_LEN    31
#define USB_BULK_CB_SIG         0x43425355    /* Spells out USBC */
#define USB_BULK_IN_FLAG        0x80

/* Command Status Wrapper */
struct bulk_cs_wrap_cdrom {
    __le32      Signature;            /* Should = 'USBS' */
    u32         Tag;                  /* Same as original command */
    __le32      Residue;              /* Amount not transferred */
    u8          Status;               /* See below */
};

#define USB_BULK_CS_WRAP_LEN    13
#define USB_BULK_CS_SIG         0x53425355    /* Spells out 'USBS' */
#define USB_STATUS_PASS         0
#define USB_STATUS_FAIL         1
#define USB_STATUS_PHASE_ERROR  2

/* Bulk-only class specific requests */
#define USB_BULK_RESET_REQUEST          0xff
#define USB_BULK_GET_MAX_LUN_REQUEST    0xfe

/* Length of a SCSI Command Data Block */
#define MAX_COMMAND_SIZE                16

/* SCSI commands that we recognize */
#define SC_FORMAT_UNIT                      0x04
#define SC_INQUIRY                          0x12
#define SC_MODE_SELECT_6                    0x15
#define SC_MODE_SELECT_10                   0x55
#define SC_MODE_SENSE_6                     0x1a
#define SC_MODE_SENSE_10                    0x5a
#define SC_PREVENT_ALLOW_MEDIUM_REMOVAL     0x1e
#define SC_READ_6                           0x08
#define SC_READ_10                          0x28
#define SC_READ_12                          0xa8
#define SC_READ_CAPACITY                    0x25
#define SC_READ_FORMAT_CAPACITIES           0x23
#define SC_RELEASE                          0x17
#define SC_REQUEST_SENSE                    0x03
#define SC_RESERVE                          0x16
#define SC_SEND_DIAGNOSTIC                  0x1d
#define SC_START_STOP_UNIT                  0x1b
#define SC_TEST_UNIT_READY                  0x00
#define SC_VERIFY                           0x2f
#define SC_READ_TOC                         0x43
#define SC_GET_CONFIGURATION                0x46
#define SC_GET_EVENT_STATUS_NOTIFICATION    0x4a
#define SC_READ_DISC_INFORMATION            0x51
#define SC_CDROM_MODE_CHANGE                0xf1
#define SC_CDROM_TIMER_CANCEL               0xf2
#define SC_CDROM_MODE_CHANGE_BS             0xf3
#define SC_GPCMD_SEND_EVENT                 0xa2

/* SCSI Sense Key/Additional Sense Code/ASC Qualifier values */
#define SS_NO_SENSE                             0
#define SS_COMMUNICATION_FAILURE                0x040800
#define SS_INVALID_COMMAND                      0x052000
#define SS_INVALID_FIELD_IN_CDB                 0x052400
#define SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE   0x052100
#define SS_LOGICAL_UNIT_NOT_SUPPORTED           0x052500
#define SS_MEDIUM_NOT_PRESENT                   0x023a00
#define SS_MEDIUM_REMOVAL_PREVENTED             0x055302
#define SS_NOT_READY_TO_READY_TRANSITION        0x062800
#define SS_RESET_OCCURRED                       0x062900
#define SS_SAVING_PARAMETERS_NOT_SUPPORTED      0x053900
#define SS_UNRECOVERED_READ_ERROR               0x031100

#define SK(x)           ((u8) ((x) >> 16))    /* Sense Key byte, etc. */
#define ASC(x)          ((u8) ((x) >> 8))
#define ASCQ(x)         ((u8) (x))

/*-------------------------------------------------------------------------*/

struct lun_cdrom {
    struct file     *filp;
    loff_t          file_length;
    loff_t          num_sectors;

    unsigned int    ro : 1;
    unsigned int    prevent_medium_removal : 1;
    unsigned int    registered : 1;
    unsigned int    info_valid : 1;

    u32             sense_data;
    u32             sense_data_info;
    u32             unit_attention_data;

    struct device   dev;
};

#define backing_file_is_open(curlun)    ((curlun)->filp != NULL)

/* Big enough to hold our biggest descriptor */
#define EP0_BUFSIZE         256

/* Number of buffers for CBW, DATA and CSW */
#define NUM_BUFFERS         4

enum fsg_buffer_state_cdrom {
    BUF_STATE_EMPTY = 0,
    BUF_STATE_FULL,
    BUF_STATE_BUSY
};

struct fsg_buffhd_cdrom {
    void                            *buf;
    enum fsg_buffer_state_cdrom     state;
    struct fsg_buffhd_cdrom         *next;

    /* The NetChip 2280 is faster, and handles some protocol faults
     * better, if we don't submit any short bulk-out read requests.
     * So we will record the intended request length here. */
    unsigned int                    bulk_out_intended_length;

    struct usb_request              *inreq;
    int                             inreq_busy;
    struct usb_request              *outreq;
    int                             outreq_busy;
};

enum fsg_state_cdrom {
    /* This one isn't used anywhere */
    FSG_STATE_COMMAND_PHASE = -10,

    FSG_STATE_DATA_PHASE,
    FSG_STATE_STATUS_PHASE,

    FSG_STATE_IDLE = 0,
    FSG_STATE_ABORT_BULK_OUT,
    FSG_STATE_RESET,
    FSG_STATE_CONFIG_CHANGE,
    FSG_STATE_EXIT,
    FSG_STATE_TERMINATED
};

enum data_direction_cdrom {
    DATA_DIR_UNKNOWN = 0,
    DATA_DIR_FROM_HOST,
    DATA_DIR_TO_HOST,
    DATA_DIR_NONE
};
int can_stall_cdrom = 1;

enum cdrom_read_flg {
    READ_DATA_IDEL = 0,
    READ_DATA_REQ,
    READ_DATA_WAIT,
    READ_DATA_RCV
};

struct fsg_dev_cdrom {
    struct usb_function         function;
    struct usb_composite_dev    *cdev;

	/* optional "usb_mass_storage" platform device */
	struct platform_device *pdev;

    /* lock protects: state and all the req_busy's */
    spinlock_t                  lock;

    /* filesem protects: backing files in use */
    struct rw_semaphore         filesem;

    /* reference counting: wait until all LUNs are released */
    struct kref                 ref;

    unsigned int                bulk_out_maxpacket;
    enum fsg_state_cdrom        state;        /* For exception handling */

    u8                          config, new_config;

    unsigned int                running : 1;
    unsigned int                bulk_in_enabled : 1;
    unsigned int                bulk_out_enabled : 1;
    unsigned int                phase_error : 1;
    unsigned int                short_packet_received : 1;
    unsigned int                bad_lun_okay : 1;

    unsigned long               atomic_bitflags;
#define REGISTERED          0
#define CLEAR_BULK_HALTS    1
#define SUSPENDED           2

    struct usb_ep               *bulk_in;
    struct usb_ep               *bulk_out;

    struct fsg_buffhd_cdrom     *next_buffhd_to_fill;
    struct fsg_buffhd_cdrom     *next_buffhd_to_drain;
    struct fsg_buffhd_cdrom     buffhds[NUM_BUFFERS];

    int                         thread_wakeup_needed;
    struct completion           thread_notifier;
    struct task_struct          *thread_task;

    int                         cmnd_size;
    u8                          cmnd[MAX_COMMAND_SIZE];
    enum data_direction_cdrom   data_dir;
    u32                         data_size;
    u32                         data_size_from_cmnd;
    u32                         tag;
    unsigned int                lun_cdrom;
    u32                         residue;
    u32                         usb_amount_left;

    unsigned int                nluns;
    struct lun_cdrom            *luns;
    struct lun_cdrom            *curlun;

    u32                         buf_size;
    const char                  *vendor;
    const char                  *product;
    int                         release;

    struct switch_dev           sdev;

    struct wake_lock            wake_lock;
};

/* Mode sense (10) command response data */
/* C/DVD Capabilities & Mechanical Status Mode Page */
static u8 eps_mode_sense10_data[24] = {
    0x2a,              /* PS(1b),Reserved(1b),Page Code(6b) */
    0x18,              /* Page Length */
    0x00,              /* Reserved(2b),           DVD-RAM Rd(1b),DVD-R Rd(1b), DVD-ROM Rd(1b),Method 2(1b),  CD-RW Rd(1b),CD-R Rd(1b) */
    0x00,              /* Reserved(2b),           DVD-RAM Wr(1b),DVD-R Wr(1b), Reserved(1b),  Test Write(1b),CD-RW Wr(1b),CD-R Wr(1b) */
    0x00,              /* BUF,       Multisession,Mode2 Form2,   Mode2 Form1,  Dig Port(2),   Dig Port(1),   Composite,   Audio Play */
    0x00,              /* Rd BurCode,UPC,         ISRC,          C2PointersSpt,R-W D&C,       R-W Support,   CDDA Stream, CD-DA */
    0x00,              /* LMT(3b),                               Reserved,     Eject,         PreventJumper, Lock State,  Lock */
    0x00,              /* Reserved(2b),           R-W in Lead-in,SideChange,   SSS,           SDP,           Separate,    Sep.vol */
    0x00, 0x00,        /* Obsolete */
    0x00, 0x00,        /* Number of Volume Levels Supported*/
    0x00, 0x00,        /* Buffer Size supported by Logical Unit(in KBytes) */
    0x00, 0x00,        /* Obsolete */
    0x00,              /* Obsolete */
    0x00,              /* Reserved(2b),           Length(2b),                  LSBF,          RCK,           BCKF,        Reserved */
    0x00, 0x00,        /* Obsolete */
    0x00, 0x00,        /* Obsolete */
    0x00, 0x00         /* Copy Management Revision Supported */
};

/* Read toc command response data */
/* Time 00 Format 00 Allocation Length 0C00 */
static u8 eps_cdrom_read_toc_time00_format00_AL0C00[12] = {
    0x00, 0x0a, 0x01, 0x01, 0x00, 0x14, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00
};

/* Read toc command response data */

static u8 eps_cdrom_read_toc_time00_format00[20] = {
    0x00, 0x12, 0x01, 0x01, 0x00, 0x14, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xaa, 0x00,
    0x00, 0x00, 0x00, 0x00  /* READ CAPACITY LBA + 1 */
};

/* Read toc command response data */
/* Time 00 Format 01 */
static u8 eps_cdrom_read_toc_time00_format01[12] = {
    0x00, 0x0a, 0x01, 0x01, 0x00, 0x14, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00
};

/* Read toc command response data */
/* Time 01 Format 00 Allocation Length 0C00 */
static u8 eps_cdrom_read_toc_time01_format00_AL0C00[12] = {
    0x00, 0x0a, 0x01, 0x01, 0x00, 0x14, 0x01, 0x00,
    0x00, 0x00, 0x02, 0x00
};

/* Read toc command response data */

static u8 eps_cdrom_read_toc_time01_format00[20] = {
    0x00, 0x12, 0x01, 0x01, 0x00, 0x14, 0x01, 0x00,
    0x00, 0x00, 0x02, 0x00, 0x00, 0x14, 0xaa, 0x00,
    0x00, 0x00, 0x0d, 0x35
};

/* Read toc command response data */
/* Time 01 Format 01 */
static u8 eps_cdrom_read_toc_time01_format01[12] = {
    0x00, 0x0a, 0x01, 0x01, 0x00, 0x14, 0x01, 0x00,
    0x00, 0x00, 0x02, 0x00
};

/* Read toc command response data */
/* read disc informatino */
static u8 eps_cdrom_read_disc_information_data[34] = {
    0x00, 0x20, 0x0e, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

static int                  readtocflg = 0;
static u32                  g_filesize = 0;
static u32                  eps_CDROM_ReadTOC_LBA; /* Max logical block - 1 */ 

static u32                  g_cdromdisk = SS_MEDIUM_NOT_PRESENT;
static u32                  g_residue = 0;
static u32                  g_offset = 0;
static enum cdrom_read_flg  g_readflg = READ_DATA_IDEL;

static int                  g_timerflg = 0; 

#define ESP_CDROM_NOMEDIA_COUNT_MAX 3 /* Max NoMedia Count */
static int                  nec_Request_Sense_sended_NoMedia_count = 0; /* NoMedia Counter */

/*-------------------------------------------------------------------------*/

static int send_status(struct fsg_dev_cdrom *fsg);

static int send_status2(struct fsg_dev_cdrom *fsg);

static int finish_reply(struct fsg_dev_cdrom *fsg);

static inline struct fsg_dev_cdrom *func_to_dev(struct usb_function *f)
{
    return container_of(f, struct fsg_dev_cdrom, function);
}

static int exception_in_progress(struct fsg_dev_cdrom *fsg)
{
    return (fsg->state > FSG_STATE_IDLE);
}

/* Make bulk-out requests be divisible by the maxpacket size */
static void set_bulk_out_req_length(struct fsg_dev_cdrom *fsg,
        struct fsg_buffhd_cdrom *bh, unsigned int length)
{
    unsigned int    rem;

    bh->bulk_out_intended_length = length;
    rem = length % fsg->bulk_out_maxpacket;
    if (rem > 0) {
        length += fsg->bulk_out_maxpacket - rem;
    }
    bh->outreq->length = length;
}

struct fsg_dev_cdrom        *the_fsg;

/*-------------------------------------------------------------------------*/


static void dump_msg(struct fsg_dev_cdrom *fsg, const char *label,
		const u8 *buf, unsigned int length)
{}

static void dump_cdb(struct fsg_dev_cdrom *fsg)
{}


static int fsg_set_halt(struct fsg_dev_cdrom *fsg, struct usb_ep *ep)
{
    const char  *name;

    if (ep == fsg->bulk_in) {
        name = "bulk-in";
	} else if (ep == fsg->bulk_out) {
        name = "bulk-out";
    } else {
        return -1;
    }

    DBG(fsg, "%s set halt\n", name);
    return usb_ep_set_halt(ep);
}

/*-------------------------------------------------------------------------*/

/* Routines for unaligned data access */
static u16 get_be16(u8 *buf)
{
    return ((u16) buf[0] << 8) | ((u16) buf[1]);
}

static u32 get_be32(u8 *buf)
{
    return ((u32) buf[0] << 24) | ((u32) buf[1] << 16) |
            ((u32) buf[2] << 8) | ((u32) buf[3]);
}

static void put_be16(u8 *buf, u16 val)
{
    buf[0] = val >> 8;
    buf[1] = val;
}

static void put_be32(u8 *buf, u32 val)
{
    buf[0] = val >> 24;
    buf[1] = val >> 16;
    buf[2] = val >> 8;
    buf[3] = val & 0xff;
}

/*-------------------------------------------------------------------------*/

/*
 * DESCRIPTORS ... most are static, but strings and (full) configuration
 * descriptors are built on demand.  Also the (static) config and interface
 * descriptors are adjusted during fsg_bind().
 */

/* There is only one interface. */
static struct usb_interface_descriptor intf_desc = {
    .bLength =              sizeof intf_desc,
    .bDescriptorType =      USB_DT_INTERFACE,

    .bNumEndpoints =        2, /* Adjusted during fsg_bind() */
    .bInterfaceClass =      USB_CLASS_MASS_STORAGE,
    .bInterfaceSubClass =   US_SC_SCSI,
    .bInterfaceProtocol =   US_PR_BULK,
};

/* Three full-speed endpoint descriptors: bulk-in, bulk-out,
 * and interrupt-in. */
static struct usb_endpoint_descriptor fs_bulk_in_desc = {
    .bLength =              USB_DT_ENDPOINT_SIZE,
    .bDescriptorType =      USB_DT_ENDPOINT,

    .bEndpointAddress =     USB_DIR_IN,
    .bmAttributes =         USB_ENDPOINT_XFER_BULK,
    /* wMaxPacketSize set by autoconfiguration */
};

static struct usb_endpoint_descriptor fs_bulk_out_desc = {
    .bLength =              USB_DT_ENDPOINT_SIZE,
    .bDescriptorType =      USB_DT_ENDPOINT,

    .bEndpointAddress =     USB_DIR_OUT,
    .bmAttributes =         USB_ENDPOINT_XFER_BULK,
    /* wMaxPacketSize set by autoconfiguration */
};

static struct usb_descriptor_header *fs_function[] = {
    (struct usb_descriptor_header *) &intf_desc,
    (struct usb_descriptor_header *) &fs_bulk_in_desc,
    (struct usb_descriptor_header *) &fs_bulk_out_desc,
    NULL,
};

#define FS_FUNCTION_PRE_EP_ENTRIES    2

static struct usb_endpoint_descriptor hs_bulk_in_desc = {
    .bLength =              USB_DT_ENDPOINT_SIZE,
    .bDescriptorType =      USB_DT_ENDPOINT,

    /* bEndpointAddress copied from fs_bulk_in_desc during fsg_bind() */
    .bmAttributes =         USB_ENDPOINT_XFER_BULK,
    .wMaxPacketSize =       __constant_cpu_to_le16(512),
};

static struct usb_endpoint_descriptor hs_bulk_out_desc = {
    .bLength =              USB_DT_ENDPOINT_SIZE,
    .bDescriptorType =      USB_DT_ENDPOINT,

    /* bEndpointAddress copied from fs_bulk_out_desc during fsg_bind() */
    .bmAttributes =         USB_ENDPOINT_XFER_BULK,
    .wMaxPacketSize =       __constant_cpu_to_le16(512),
    .bInterval =            1,    /* NAK every 1 uframe */
};


static struct usb_descriptor_header *hs_function[] = {
    (struct usb_descriptor_header *) &intf_desc,
    (struct usb_descriptor_header *) &hs_bulk_in_desc,
    (struct usb_descriptor_header *) &hs_bulk_out_desc,
    NULL,
};

/* Maxpacket and other transfer characteristics vary by speed. */
static struct usb_endpoint_descriptor *
ep_desc(struct usb_gadget *g, struct usb_endpoint_descriptor *fs,
        struct usb_endpoint_descriptor *hs)
{
    if (gadget_is_dualspeed(g) && g->speed == USB_SPEED_HIGH) {
        return hs;
    }
    return fs;
}

/*-------------------------------------------------------------------------*/

/* These routines may be called in process context or in_irq */

/* Caller must hold fsg->lock */
static void wakeup_thread(struct fsg_dev_cdrom *fsg)
{
    /* Tell the main thread that something has happened */
    fsg->thread_wakeup_needed = 1;
    if (fsg->thread_task) {
        wake_up_process(fsg->thread_task);
    }
}


static void raise_exception(struct fsg_dev_cdrom *fsg, enum fsg_state_cdrom new_state)
{
    unsigned long        flags;

    DBG(fsg, "raise_exception %d\n", (int)new_state);
    /* Do nothing if a higher-priority exception is already in progress.
     * If a lower-or-equal priority exception is in progress, preempt it
     * and notify the main thread by sending it a signal. */
    spin_lock_irqsave(&fsg->lock, flags);
    if (fsg->state <= new_state) {
        fsg->state = new_state;
        if (fsg->thread_task) {
            send_sig_info(SIGUSR1, SEND_SIG_FORCED,
                    fsg->thread_task);
        }
    }
    spin_unlock_irqrestore(&fsg->lock, flags);
}

/*-------------------------------------------------------------------------*/

/* Bulk and interrupt endpoint completion handlers.
 * These always run in_irq. */
static void bulk_in_complete(struct usb_ep *ep, struct usb_request *req)
{
    struct fsg_dev_cdrom        *fsg = ep->driver_data;
    struct fsg_buffhd_cdrom     *bh = req->context;

	if (req->status || req->actual != req->length) {
		DBG(fsg, "%s --> %d, %u/%u\n", __func__,
				req->status, req->actual, req->length);
    }

    /* Hold the lock while we update the request and buffer states */
    smp_wmb();
    spin_lock(&fsg->lock);
    bh->inreq_busy = 0;
    bh->state = BUF_STATE_EMPTY;
    wakeup_thread(fsg);
    spin_unlock(&fsg->lock);
}

static void bulk_out_complete(struct usb_ep *ep, struct usb_request *req)
{
    struct fsg_dev_cdrom        *fsg = ep->driver_data;
    struct fsg_buffhd_cdrom     *bh = req->context;

    dump_msg(fsg, "bulk-out", req->buf, req->actual);
    if (req->status || req->actual != bh->bulk_out_intended_length) {
        DBG(fsg, "%s --> %d, %u/%u\n", __func__,
                req->status, req->actual,
                bh->bulk_out_intended_length);
    }

    /* Hold the lock while we update the request and buffer states */
    smp_wmb();
    spin_lock(&fsg->lock);
    bh->outreq_busy = 0;
    bh->state = BUF_STATE_FULL;
    wakeup_thread(fsg);
    spin_unlock(&fsg->lock);
}

static int fsg_function_setup(struct usb_function *f,
                    const struct usb_ctrlrequest *ctrl)
{
    struct fsg_dev_cdrom        *fsg = func_to_dev(f);
    struct usb_composite_dev    *cdev = fsg->cdev;
    int                         value = -EOPNOTSUPP;
    u16                         w_index = le16_to_cpu(ctrl->wIndex);
    u16                         w_value = le16_to_cpu(ctrl->wValue);
    u16                         w_length = le16_to_cpu(ctrl->wLength);

    DBG(fsg, "fsg_function_setup\n");

    /* Handle Bulk-only class-specific requests */
    if ((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_CLASS) {
        DBG(fsg, "USB_TYPE_CLASS\n");
        switch (ctrl->bRequest) {
        case USB_BULK_RESET_REQUEST:
            if (ctrl->bRequestType != (USB_DIR_OUT |
                    USB_TYPE_CLASS | USB_RECIP_INTERFACE)) {
                break;
            }
            if (w_value != 0) {
                value = -EDOM;
                break;
            }

            /* Raise an exception to stop the current operation
             * and reinitialize our state. */
            DBG(fsg, "bulk reset request\n");
            raise_exception(fsg, FSG_STATE_RESET);
            value = 0;
            break;

        case USB_BULK_GET_MAX_LUN_REQUEST:
            if (ctrl->bRequestType != (USB_DIR_IN |
                    USB_TYPE_CLASS | USB_RECIP_INTERFACE)) {
                break;
            }
            if (w_value != 0) {
                value = -EDOM;
                break;
            }
            VDBG(fsg, "get max LUN\n");
            *(u8 *)cdev->req->buf = fsg->nluns - 1;
            value = 1;
            break;
        }
    }

    /* respond with data transfer or status phase? */
    if (value >= 0) {
        int rc;
        cdev->req->zero = value < w_length;
        cdev->req->length = value;
        rc = usb_ep_queue(cdev->gadget->ep0, cdev->req, GFP_ATOMIC);
        if (rc < 0) {
            printk("%s setup response queue error\n", __func__);
        }
    }

    if (value == -EOPNOTSUPP) {
        VDBG(fsg,
            "unknown class-specific control req "
            "%02x.%02x v%04x i%04x l%u\n",
            ctrl->bRequestType, ctrl->bRequest,
            le16_to_cpu(ctrl->wValue), w_index, w_length);
    }
    return value;
}

/*-------------------------------------------------------------------------*/

/* All the following routines run in process context */

/* Use this for bulk or interrupt transfers, not ep0 */
static void start_transfer(struct fsg_dev_cdrom *fsg, struct usb_ep *ep,
        struct usb_request *req, int *pbusy,
        enum fsg_buffer_state_cdrom *state)
{
    int                 rc;

    DBG(fsg, "start_transfer req: %p, req->buf: %p\n", req, req->buf);
    if (ep == fsg->bulk_in) {
        dump_msg(fsg, "bulk-in", req->buf, req->length);
    }

    spin_lock_irq(&fsg->lock);
    *pbusy = 1;
    *state = BUF_STATE_BUSY;
    spin_unlock_irq(&fsg->lock);
    rc = usb_ep_queue(ep, req, GFP_KERNEL);
    if (rc != 0) {
        *pbusy = 0;
        *state = BUF_STATE_EMPTY;

        /* We can't do much more than wait for a reset */

    	comm_usb_cdrom_interruptible();


        /* Note: currently the net2280 driver fails zero-length
         * submissions if DMA is enabled. */
        if (rc != -ESHUTDOWN && !(rc == -EOPNOTSUPP && req->length == 0)) {
            WARN(fsg, "error in submission: %s --> %d\n",
                (ep == fsg->bulk_in ? "bulk-in" : "bulk-out"),
                rc);
        }
    }
}


static int	f_cdrom_interrupt = 0;

void cdrom_mode_interrupt(void)
{
	printk(KERN_INFO "%s\n", __func__);
	f_cdrom_interrupt = 1;
}

void cdrom_mode_interrupt_cleanup(void)
{
	printk(KERN_INFO "%s\n", __func__);
	f_cdrom_interrupt = 0;
}


static int sleep_thread(struct fsg_dev_cdrom *fsg)
{
    int                 rc = 0;

    /* Wait until a signal arrives or we are woken up */
	for (;;) {
        try_to_freeze();
        set_current_state(TASK_INTERRUPTIBLE);
        if(signal_pending(current)) {
            rc = -EINTR;
            break;
        }
        if (fsg->thread_wakeup_needed) {
            break;
        }
        schedule();
    }
    __set_current_state(TASK_RUNNING);
    fsg->thread_wakeup_needed = 0;
    return rc;
}


static int sleep_thread2(struct fsg_dev_cdrom *fsg)
{
    int                 rc = 0;

    /* Wait until a signal arrives or we are woken up */
    for (;;) {
        try_to_freeze();
        if (f_cdrom_interrupt) {
            printk(KERN_ERR "%s: f_cdrom_interrupt=1\n", __func__);
            rc = -EINTR;
            break;
        }
        if(signal_pending(current)) {
            printk(KERN_ERR "%s: signal_pending\n", __func__);
            rc = -EINTR;
            break;
        }

    	if(fsg->next_buffhd_to_fill->state == BUF_STATE_EMPTY){
	        printk(KERN_ERR "%s:fsg->next_buffhd_to_fill->state = BUF_STATE_EMPTY\n", __func__);
       	    break;
    	}

        schedule();
    }
    f_cdrom_interrupt = 0;
    return rc;
}


/*-------------------------------------------------------------------------*/
int send_cdrom_data(u32 size, u8 *data)
{

    struct fsg_dev_cdrom        *fsg;
    struct fsg_buffhd_cdrom     *bh;
    u8                          *buf;
    int                         rc;
    u32                         amount;
    unsigned long               flags;
    
    if(!data) {
        printk(KERN_ERR "%s ERROR: *data is NULL pointer.\n", __func__);
        return -1;
    }
    
    if(!the_fsg) {
        printk(KERN_ERR "%s ERROR: *the_fsg is NULL pointer.\n", __func__);
        return -1;
    }
    
    fsg = the_fsg;
    bh = fsg->next_buffhd_to_fill;
    
    if(!bh) {
        printk(KERN_ERR "%s ERROR: *bh is NULL pointer.\n", __func__);
        return -1;
    }
    


    while (bh->state != BUF_STATE_EMPTY) {

        rc = sleep_thread2(fsg);

        if (rc) {
            printk(KERN_ERR "%s sleep_thread() failed : %d\n", __func__, rc);
            wakeup_thread(fsg); 
            return rc;
        }
    }
	

    buf = (u8 *) bh->buf;
    
    if(!buf) {
        printk(KERN_ERR "%s ERROR: *buf is NULL pointer.\n", __func__);
        return -1;
    }


    memset(buf, 0, size);

    
    memcpy(buf, data, size);


    if(!bh->inreq) {
        printk(KERN_ERR "%s ERROR: *bh->inreq is NULL pointer.\n", __func__);
        return -1;
    }


   
    bh->inreq->length = min(size, fsg->data_size_from_cmnd);

    fsg->data_dir = DATA_DIR_TO_HOST;
    fsg->residue = 0;
    g_readflg = READ_DATA_RCV;

    start_transfer(fsg, fsg->bulk_in, bh->inreq, &bh->inreq_busy, &bh->state);
    fsg->next_buffhd_to_fill = bh->next;

    g_residue -= size;
    if (g_residue <= 0) {
        g_offset = 0;
        g_residue = 0;

        rc = send_status2(fsg);
        if (rc) {
            printk(KERN_ERR "%s send_status2() failed : %d\n", __func__, rc);

            return -1;

        }

        g_readflg = READ_DATA_IDEL;

        spin_lock_irqsave(&fsg->lock, flags);
        wakeup_thread(fsg);
        spin_unlock_irqrestore(&fsg->lock, flags);
    } else {
        
        amount = min((unsigned int) g_residue, (unsigned int)fsg->buf_size);
        g_offset += size;
        g_readflg = READ_DATA_REQ;
        rc = comm_usb_request_read(g_offset, amount);
        if (rc) {
            printk(KERN_ERR "%s comm_usb_request_read() failed : %d\n", __func__, rc);
        }
    }

    return 0;
}

static int do_read(struct fsg_dev_cdrom *fsg)
{
    u32                 lba;
    int                 rc;
    u32                 amount_left;
    u32                 file_offset;
    unsigned int        amount;


    /* Get the starting Logical Block Address and check that it's
     * not too big */
    if (fsg->cmnd[0] == SC_READ_6) {
        lba = (fsg->cmnd[1] << 16) | get_be16(&fsg->cmnd[2]);
    } else {
        lba = get_be32(&fsg->cmnd[2]);
    }
    file_offset = g_offset = lba << 11;

    g_residue = fsg->data_size_from_cmnd;

    /* Carry out the file reads */
    amount_left = fsg->data_size_from_cmnd;

    amount = min((unsigned int) amount_left, (unsigned int)fsg->buf_size);

    /* Perform the read */
    fsg->data_dir = DATA_DIR_NONE;
    g_readflg = READ_DATA_REQ;

    rc = comm_usb_request_read(file_offset, amount);
    if(rc)
    {
        printk(KERN_ERR "%s comm_usb_request_read() failed : %d\n", __func__, rc);
    }

    return 0;
}


/*-------------------------------------------------------------------------*/

static int do_inquiry(struct fsg_dev_cdrom *fsg, struct fsg_buffhd_cdrom *bh)
{
    u8      *buf = (u8 *) bh->buf;

    if (!fsg->curlun) {		/* Unsupported LUNs are okay */
        fsg->bad_lun_okay = 1;
        memset(buf, 0, 36);
        buf[0] = 0x7f;        /* Unsupported, no device-type */
        return 36;
    }

    memset(buf, 0, 36);    /* Non-removable, direct-access device */

    buf[0] = 0x05;    /* set Peripheral device type bit */
    buf[1] = 0x80;    /* set removable bit */
    buf[2] = 2;        /* ANSI SCSI level 2 */
    buf[3] = 0;        /* SCSI-1 INQUIRY data format */
    buf[4] = 31;        /* Additional length */

    /* No special options */
    buf[8] = 0x43;
    buf[9] = 0x41;
    buf[10] = 0x53;
    buf[11] = 0x49;
    buf[12] = 0x4f;
    buf[13] = 0x20;
    buf[14] = 0x20;
    buf[15] = 0x20;
    
    buf[16] = 0x43;
    buf[17] = 0x44;
    buf[18] = 0x52;
    buf[19] = 0x4f;
    buf[20] = 0x4d;
    buf[21] = 0x20;
    buf[22] = 0x20;
    buf[23] = 0x20;
    buf[24] = 0x20;
    buf[25] = 0x20;
    buf[26] = 0x20;
    buf[27] = 0x20;
    buf[28] = 0x20;
    buf[29] = 0x20;
    buf[30] = 0x20;
    buf[31] = 0x20;
    
    buf[32] = 0x31;
    buf[33] = 0x2e;
    buf[34] = 0x30;
    buf[35] = 0x30;

    return 36;
}


static int do_request_sense(struct fsg_dev_cdrom *fsg, struct fsg_buffhd_cdrom *bh)
{
    struct lun_cdrom    *curlun = fsg->curlun;
    u8                  *buf = (u8 *) bh->buf;
    u32                 sd, sdinfo;
    int                 valid;

    /*
     * From the SCSI-2 spec., section 7.9 (Unit attention condition):
     *
     * If a REQUEST SENSE command is received from an initiator
     * with a pending unit attention condition (before the target
     * generates the contingent allegiance condition), then the
     * target shall either:
     *   a) report any pending sense data and preserve the unit
     *    attention condition on the logical unit, or,
     *   b) report the unit attention condition, may discard any
     *    pending sense data, and clear the unit attention
     *    condition on the logical unit for that initiator.
     *
     * FSG normally uses option a); enable this code to use option b).
     */
    if (!curlun) {		/* Unsupported LUNs are okay */
        fsg->bad_lun_okay = 1;
        sd = SS_LOGICAL_UNIT_NOT_SUPPORTED;
        sdinfo = 0;
        valid = 0;
    } else {
        sd = curlun->sense_data;
        sdinfo = curlun->sense_data_info;
        valid = curlun->info_valid << 7;
        curlun->sense_data = SS_NO_SENSE;
        curlun->sense_data_info = 0;
        curlun->info_valid = 0;
   }

    
    
    if (g_cdromdisk == SS_MEDIUM_NOT_PRESENT) {
        
        
        nec_Request_Sense_sended_NoMedia_count ++;
        
        if ( ESP_CDROM_NOMEDIA_COUNT_MAX < nec_Request_Sense_sended_NoMedia_count) {
            
            
            
            sd = SS_NOT_READY_TO_READY_TRANSITION;
            g_cdromdisk = SS_NOT_READY_TO_READY_TRANSITION;

            
            nec_Request_Sense_sended_NoMedia_count = 0;
        } else {
            sd = SS_MEDIUM_NOT_PRESENT;
        }
    }

    memset(buf, 0, 18);
    buf[0] = valid | 0x70;          /* Valid, current error */
    buf[2] = SK(sd);
    put_be32(&buf[3], sdinfo);      /* Sense information */
    buf[7] = 18 - 8;                /* Additional sense length */
    buf[12] = ASC(sd);
    buf[13] = ASCQ(sd);

    return 18;
}


static int do_read_capacity(struct fsg_dev_cdrom *fsg, struct fsg_buffhd_cdrom *bh)
{
    struct lun_cdrom    *curlun = fsg->curlun;
    u32                 lba = get_be32(&fsg->cmnd[2]);
    int                 pmi = fsg->cmnd[8];
    u8                  *buf = (u8 *) bh->buf;

    /* Check the PMI and LBA fields */
    if (pmi > 1 || (pmi == 0 && lba != 0)) {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        return -EINVAL;
    }

    curlun->num_sectors = g_filesize >> 11; 
    put_be32(&buf[0], curlun->num_sectors);    /* Max logical block */
    put_be32(&buf[4], 2048);                /* Block length */

    eps_CDROM_ReadTOC_LBA = curlun->num_sectors - 1;

    return 8;
}


static int do_mode_sense(struct fsg_dev_cdrom *fsg, struct fsg_buffhd_cdrom *bh)
{
    struct lun_cdrom    *curlun = fsg->curlun;
    int         mscmnd = fsg->cmnd[0];
    u8          *buf = (u8 *) bh->buf;
    u8          *buf0 = buf;
    int         pc, page_code;
    int         changeable_values, all_pages;
    int         valid_page = 0;
    int         len, limit;

    if ((fsg->cmnd[1] & ~0x08) != 0) {        /* Mask away DBD */
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        return -EINVAL;
    }
    pc = fsg->cmnd[2] >> 6;
    page_code = fsg->cmnd[2] & 0x3f;
    if (pc == 3) {
        curlun->sense_data = SS_SAVING_PARAMETERS_NOT_SUPPORTED;
        return -EINVAL;
    }
    changeable_values = (pc == 1);
    all_pages = (page_code == 0x3f);

    /* Write the mode parameter header.  Fixed values are: default
     * medium type, no cache control (DPOFUA), and no block descriptors.
     * The only variable value is the WriteProtect bit.  We will fill in
     * the mode data length later. */
    if (mscmnd == SC_MODE_SENSE_6) {
        memset(buf, 0, 8);
        buf[2] = (curlun->ro ? 0x80 : 0x00);        /* WP, DPOFUA */
        buf += 4;
        limit = 255;
    } else {            /* SC_MODE_SENSE_10 */
        memset(buf, 0, 24);
        buf[3] = (curlun->ro ? 0x80 : 0x00);        /* WP, DPOFUA */
        buf += 8;
        memcpy(buf, eps_mode_sense10_data, sizeof(eps_mode_sense10_data));
        buf += 24;
        limit = 65535;
    }

    /* No block descriptors */

    /* Disabled to workaround USB reset problems with a Vista host.
     */
    valid_page = 1;

    /* Check that a valid page was requested and the mode data length
     * isn't too long. */
    len = buf - buf0;
    if (!valid_page || len > limit) {
        curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
        return -EINVAL;
    }

    /*  Store the mode data length */
    if (mscmnd == SC_MODE_SENSE_6) {
        buf0[0] = len - 1;
    } else {
        put_be16(buf0, len - 2);
    }

    return len;
}


static int do_mode_select(struct fsg_dev_cdrom *fsg, struct fsg_buffhd_cdrom *bh)
{
    struct lun_cdrom    *curlun = fsg->curlun;

    /* We don't support MODE SELECT */
    curlun->sense_data = SS_INVALID_COMMAND;
    return -EINVAL;
}


static int do_read_toc(struct fsg_dev_cdrom *fsg, struct fsg_buffhd_cdrom *bh)
{
    u8          *buf = (u8 *) bh->buf;
    int         len;
    int         cnt=0;

    switch(fsg->cmnd[1]) {
    /* Time 00 */
    case 0x00:
        switch(fsg->cmnd[2]) {
        /* Format 00 */
        case 0x00:
            
            if((0x00 == fsg->cmnd[7]) && (0x0C == fsg->cmnd[8])) {
                memset(buf, 0, sizeof(eps_cdrom_read_toc_time00_format00_AL0C00));
                memcpy(buf, eps_cdrom_read_toc_time00_format00_AL0C00, sizeof(eps_cdrom_read_toc_time00_format00_AL0C00));
                cnt += 12;
            } else {
                
                
                eps_cdrom_read_toc_time00_format00[16] = (unsigned char)(((eps_CDROM_ReadTOC_LBA)) >> 24);
                eps_cdrom_read_toc_time00_format00[17] = (unsigned char)(((eps_CDROM_ReadTOC_LBA)) >> 16);
                eps_cdrom_read_toc_time00_format00[18] = (unsigned char)(((eps_CDROM_ReadTOC_LBA)) >> 8);
                eps_cdrom_read_toc_time00_format00[19] = (unsigned char)((eps_CDROM_ReadTOC_LBA));

                memset(buf, 0, sizeof(eps_cdrom_read_toc_time00_format00));
                memcpy(buf, eps_cdrom_read_toc_time00_format00, sizeof(eps_cdrom_read_toc_time00_format00));
                cnt += 20;
            }
            break;

        /* Format 01 */
        case 0x01:
            memset(buf, 0, sizeof(eps_cdrom_read_toc_time00_format01));
            memcpy(buf, eps_cdrom_read_toc_time00_format01, sizeof(eps_cdrom_read_toc_time00_format01));
            cnt += 12;
            break;

        default:
            
            printk(KERN_ERR "%s READ_TOC include non support format command.\n", __func__);
            break;
        }
        break;

    /* Time 01 */
    case 0x02:
        switch(fsg->cmnd[2]) {
        /* Format 00 */
        case 0x00:
            
            if((0x00 == fsg->cmnd[7]) && (0x0C == fsg->cmnd[8])) {
                memset(buf, 0, sizeof(eps_cdrom_read_toc_time01_format00_AL0C00));
                memcpy(buf, eps_cdrom_read_toc_time01_format00_AL0C00, sizeof(eps_cdrom_read_toc_time01_format00_AL0C00));
                cnt += 12;
            } else {
                
                memset(buf, 0, sizeof(eps_cdrom_read_toc_time01_format00));
                memcpy(buf, eps_cdrom_read_toc_time01_format00, sizeof(eps_cdrom_read_toc_time01_format00));
                cnt += 20;
            }
            break;

        /* Format 01 */
        case 0x01:
            memset(buf, 0, sizeof(eps_cdrom_read_toc_time01_format01));
            memcpy(buf, eps_cdrom_read_toc_time01_format01, sizeof(eps_cdrom_read_toc_time01_format01));
            cnt += 12;
            break;

        default:
            
            printk(KERN_ERR "%s READ_TOC include non support format command.\n", __func__);
            break;
        }
        break;

    default:
        
        printk(KERN_ERR "%s READ_TOC include non support time command.\n", __func__);
        break;
    }
    
    len = cnt;
    
    
    
    readtocflg = 1;
    return len;
}


static int do_get_configuration(struct fsg_dev_cdrom *fsg, struct fsg_buffhd_cdrom *bh)
{
    u8    *buf = (u8 *) bh->buf;
    
    memset(buf, 0, 8);
    
    return 8;
}


static int do_get_event_status_notification(struct fsg_dev_cdrom *fsg, struct fsg_buffhd_cdrom *bh)
{
    u8    *buf = (u8 *) bh->buf;
    
    memset(buf, 0, 8);
    
    return 8;
}


static int do_read_disc_information(struct fsg_dev_cdrom *fsg, struct fsg_buffhd_cdrom *bh)
{
    u8        *buf = (u8 *) bh->buf;
    
    memset(buf, 0, 34);
    memcpy(buf, eps_cdrom_read_disc_information_data, sizeof(eps_cdrom_read_disc_information_data));
    
    return 34;
}

static int do_cdrom_mode_change(struct fsg_dev_cdrom *fsg)
{
    int ret;
    
    ret = comm_usb_request_close(USBMODE_CDROM);
    if (ret) {
        printk(KERN_ERR "%s comm_usb_request_close() failed.\n", __func__);
    }
    return 0;
}

static int do_cdrom_timer_cancel(struct fsg_dev_cdrom *fsg)
{
    
    printk(KERN_DEBUG "%s: @@ f_timer_end @@\n", __func__);
    f_timer_end();
    return 0;
}


static int halt_bulk_in_endpoint(struct fsg_dev_cdrom *fsg)
{
    int     rc;

    rc = fsg_set_halt(fsg, fsg->bulk_in);
    if (rc == -EAGAIN) {
        DBG(fsg, "delayed bulk-in endpoint halt\n");
    }
    while (rc != 0) {
        if (rc != -EAGAIN) {
            DBG(fsg, "usb_ep_set_halt -> %d\n", rc);
            rc = 0;
            break;
        }
        /* Wait for a short time and then try again */
        if (msleep_interruptible(100) != 0) {
            return -EINTR;
        }
        rc = usb_ep_set_halt(fsg->bulk_in);
    }
    return rc;
}
/*-------------------------------------------------------------------------*/

static int throw_away_data(struct fsg_dev_cdrom *fsg)
{
    struct fsg_buffhd_cdrom     *bh;
    u32                         amount;
    int                         rc;

    DBG(fsg, "throw_away_data\n");
    while ((bh = fsg->next_buffhd_to_drain)->state != BUF_STATE_EMPTY ||
            fsg->usb_amount_left > 0) {

        /* Throw away the data in a filled buffer */
        if (bh->state == BUF_STATE_FULL) {
            smp_rmb();
            bh->state = BUF_STATE_EMPTY;
            fsg->next_buffhd_to_drain = bh->next;

            /* A short packet or an error ends everything */
            if (bh->outreq->actual != bh->outreq->length ||
                    bh->outreq->status != 0) {
                raise_exception(fsg, FSG_STATE_ABORT_BULK_OUT);
                return -EINTR;
            }
            continue;
        }

        /* Try to submit another request if we need one */
        bh = fsg->next_buffhd_to_fill;
        if (bh->state == BUF_STATE_EMPTY && fsg->usb_amount_left > 0) {
            amount = min(fsg->usb_amount_left, (u32) fsg->buf_size);

            /* amount is always divisible by 512, hence by
             * the bulk-out maxpacket size */
            bh->outreq->length = bh->bulk_out_intended_length =
                    amount;
            start_transfer(fsg, fsg->bulk_out, bh->outreq,
                    &bh->outreq_busy, &bh->state);
            fsg->next_buffhd_to_fill = bh->next;
            fsg->usb_amount_left -= amount;
            continue;
        }

        /* Otherwise wait for something to happen */
        rc = sleep_thread(fsg);
        if (rc) {
            printk(KERN_ERR "%s sleep_thread() failed : %d\n", __func__, rc);
            return rc;
        }
    }
    return 0;
}

static int finish_reply(struct fsg_dev_cdrom *fsg)
{
	struct fsg_buffhd_cdrom     *bh = fsg->next_buffhd_to_fill;
    int                         rc = 0;
    int                         i;

    switch (fsg->data_dir) {
    case DATA_DIR_NONE:
        break;            /* Nothing to send */

    case DATA_DIR_UNKNOWN:
        rc = -EINVAL;
        break;

    /* All but the last buffer of data must have already been sent */
    case DATA_DIR_TO_HOST:
        if (fsg->data_size == 0) {
            /* Nothing to send */
        } else if (fsg->residue == 0) {
            /* If there's no residue, simply send the last buffer */
            start_transfer(fsg, fsg->bulk_in, bh->inreq,
                    &bh->inreq_busy, &bh->state);
            fsg->next_buffhd_to_fill = bh->next;
        } else {
            if (can_stall_cdrom) {
                bh->state = BUF_STATE_EMPTY;
                for (i = 0; i < NUM_BUFFERS; ++i) {
                    struct fsg_buffhd_cdrom
                            *bh = &fsg->buffhds[i];
                    while (bh->state != BUF_STATE_EMPTY) {
                        rc = sleep_thread(fsg);
                        if (rc) {
                            printk(KERN_ERR "%s sleep_thread() failed : %d\n", __func__, rc);
                            return rc;
                        }
                    }
                }
                rc = halt_bulk_in_endpoint(fsg);
            } else {
            start_transfer(fsg, fsg->bulk_in, bh->inreq,
                    &bh->inreq_busy, &bh->state);
            fsg->next_buffhd_to_fill = bh->next;
            }
        }
        break;

    /* We have processed all we want from the data the host has sent.
     * There may still be outstanding bulk-out requests. */
    case DATA_DIR_FROM_HOST:
        if (fsg->residue == 0) {
            /* Nothing to receive */
        } else if (fsg->short_packet_received) {
            /* Did the host stop sending unexpectedly early? */
            raise_exception(fsg, FSG_STATE_ABORT_BULK_OUT);
            rc = -EINTR;
        } else {
            /* We haven't processed all the incoming data.  Even though
             * we may be allowed to stall, doing so would cause a race.
             * The controller may already have ACK'ed all the remaining
             * bulk-out packets, in which case the host wouldn't see a
             * STALL.  Not realizing the endpoint was halted, it wouldn't
             * clear the halt -- leading to problems later on. */
    
            /* We can't stall.  Read in the excess data and throw it
             * all away. */
            rc = throw_away_data(fsg);
        }
        break;
    }
    return rc;
}

static int send_status(struct fsg_dev_cdrom *fsg)
{
    struct lun_cdrom        *curlun = fsg->curlun;
    struct fsg_buffhd_cdrom    *bh;
    int            rc;
    u8            status = USB_STATUS_PASS;
    u32            sd, sdinfo = 0;
    struct bulk_cs_wrap_cdrom    *csw;

    DBG(fsg, "send_status\n");

    /* Wait for the next buffer to become available */
    bh = fsg->next_buffhd_to_fill;
    while (bh->state != BUF_STATE_EMPTY) {
        rc = sleep_thread(fsg);
        if (rc) {
            printk(KERN_ERR "%s sleep_thread() failed : %d\n", __func__, rc);
            return rc;
        }
    }

    if (curlun) {
        sd = curlun->sense_data;
        sdinfo = curlun->sense_data_info;
    } else if (fsg->bad_lun_okay) {
        sd = SS_NO_SENSE;
    } else {
        sd = SS_LOGICAL_UNIT_NOT_SUPPORTED;
    }

    if (fsg->phase_error) {
        DBG(fsg, "sending phase-error status\n");
        status = USB_STATUS_PHASE_ERROR;
        sd = SS_INVALID_COMMAND;
    } else if (sd != SS_NO_SENSE) {
        DBG(fsg, "sending command-failure status\n");
        status = USB_STATUS_FAIL;
        VDBG(fsg, "  sense data: SK x%02x, ASC x%02x, ASCQ x%02x;"
                "  info x%x\n",
                SK(sd), ASC(sd), ASCQ(sd), sdinfo);
    }

    csw = bh->buf;

    /* Store and send the Bulk-only CSW */
    csw->Signature = __constant_cpu_to_le32(USB_BULK_CS_SIG);
    csw->Tag = fsg->tag;
    csw->Residue = cpu_to_le32(fsg->residue);
    csw->Status = status;

    bh->inreq->length = USB_BULK_CS_WRAP_LEN;
    start_transfer(fsg, fsg->bulk_in, bh->inreq,
            &bh->inreq_busy, &bh->state);

    fsg->next_buffhd_to_fill = bh->next;
    return 0;
}


static int send_status2(struct fsg_dev_cdrom *fsg)
{
    struct lun_cdrom        *curlun = fsg->curlun;
    struct fsg_buffhd_cdrom    *bh;
    int            rc;
    u8            status = USB_STATUS_PASS;
    u32            sd, sdinfo = 0;
    struct bulk_cs_wrap_cdrom    *csw;

    DBG(fsg, "send_status\n");

    /* Wait for the next buffer to become available */
    bh = fsg->next_buffhd_to_fill;


    if(!bh) {
        printk(KERN_ERR "%s ERROR: *csw is NULL pointer.\n", __func__);
        return -1;
    }


    while (bh->state != BUF_STATE_EMPTY) {
        rc = sleep_thread2(fsg);
        if (rc) {
            printk(KERN_ERR "%s sleep_thread() failed : %d\n", __func__, rc);
            return rc;
        }
    }

    if (curlun) {
        sd = curlun->sense_data;
        sdinfo = curlun->sense_data_info;
    } else if (fsg->bad_lun_okay) {
        sd = SS_NO_SENSE;
    } else {
        sd = SS_LOGICAL_UNIT_NOT_SUPPORTED;
    }

    if (fsg->phase_error) {
        DBG(fsg, "sending phase-error status\n");
        status = USB_STATUS_PHASE_ERROR;
        sd = SS_INVALID_COMMAND;
    } else if (sd != SS_NO_SENSE) {
        DBG(fsg, "sending command-failure status\n");
        status = USB_STATUS_FAIL;
        VDBG(fsg, "  sense data: SK x%02x, ASC x%02x, ASCQ x%02x;"
                "  info x%x\n",
                SK(sd), ASC(sd), ASCQ(sd), sdinfo);
    }

    csw = bh->buf;


    if(!csw) {
        printk(KERN_ERR "%s ERROR: *csw is NULL pointer.\n", __func__);
        return -1;
    }


    /* Store and send the Bulk-only CSW */
    csw->Signature = __constant_cpu_to_le32(USB_BULK_CS_SIG);
    csw->Tag = fsg->tag;
    csw->Residue = cpu_to_le32(fsg->residue);
    csw->Status = status;


    if(!bh->inreq) {
        printk(KERN_ERR "%s ERROR: *bh->inreq is NULL pointer.\n", __func__);
        return -1;
    }


    bh->inreq->length = USB_BULK_CS_WRAP_LEN;
    start_transfer(fsg, fsg->bulk_in, bh->inreq,
            &bh->inreq_busy, &bh->state);

    fsg->next_buffhd_to_fill = bh->next;
    return 0;
}


/*-------------------------------------------------------------------------*/

/* Check whether the command is properly formed and whether its data size
 * and direction agree with the values we already have. */
static int check_command(struct fsg_dev_cdrom *fsg, int cmnd_size,
        enum data_direction_cdrom data_dir, unsigned int mask,
        int needs_medium, const char *name)
{
    int            i;
    int            lun_cdrom = fsg->cmnd[1] >> 5;
    static const char    dirletter[4] = {'u', 'o', 'i', 'n'};
    char            hdlen[20];
    struct lun_cdrom        *curlun;

    hdlen[0] = 0;
    if (fsg->data_dir != DATA_DIR_UNKNOWN)
        sprintf(hdlen, ", H%c=%u", dirletter[(int) fsg->data_dir],
                fsg->data_size);
    VDBG(fsg, "SCSI command: %s;  Dc=%d, D%c=%u;  Hc=%d%s\n",
            name, cmnd_size, dirletter[(int) data_dir],
            fsg->data_size_from_cmnd, fsg->cmnd_size, hdlen);

    /* We can't reply at all until we know the correct data direction
     * and size. */
    if (fsg->data_size_from_cmnd == 0)
        data_dir = DATA_DIR_NONE;
    if (fsg->data_dir == DATA_DIR_UNKNOWN) {    /* CB or CBI */
        fsg->data_dir = data_dir;
        fsg->data_size = fsg->data_size_from_cmnd;

    } else {                    /* Bulk-only */
        if (fsg->data_size < fsg->data_size_from_cmnd) {

            /* Host data size < Device data size is a phase error.
             * Carry out the command, but only transfer as much
             * as we are allowed. */
            DBG(fsg, "phase error 1\n");
            fsg->data_size_from_cmnd = fsg->data_size;
            fsg->phase_error = 1;
        }
    }
    fsg->residue = fsg->usb_amount_left = fsg->data_size;

    /* Conflicting data directions is a phase error */
    if (fsg->data_dir != data_dir && fsg->data_size_from_cmnd > 0) {
        fsg->phase_error = 1;
        DBG(fsg, "phase error 2\n");
        return -EINVAL;
    }

    /* Verify the length of the command itself */
    if (cmnd_size != fsg->cmnd_size) {
        /* Special case workaround: MS-Windows issues REQUEST SENSE/
         * INQUIRY with cbw->Length == 12 (it should be 6). */
        if ((fsg->cmnd[0] == SC_REQUEST_SENSE && fsg->cmnd_size == 12)
         || (fsg->cmnd[0] == SC_INQUIRY && fsg->cmnd_size == 12))
        {
            cmnd_size = fsg->cmnd_size;
        } else {
            fsg->phase_error = 1;
            return -EINVAL;
        }
    }

    /* Check that the LUN values are consistent */
    if (fsg->lun_cdrom != lun_cdrom) {
        DBG(fsg, "using LUN %d from CBW, "
                "not LUN %d from CDB\n",
                fsg->lun_cdrom, lun_cdrom);
    }

    /* Check the LUN */
    if (fsg->lun_cdrom >= 0 && fsg->lun_cdrom < fsg->nluns) {
        fsg->curlun = curlun = &fsg->luns[fsg->lun_cdrom];
        if (fsg->cmnd[0] != SC_REQUEST_SENSE) {
            curlun->sense_data = SS_NO_SENSE;
            curlun->sense_data_info = 0;
            curlun->info_valid = 0;
        }
    } else {
	    printk(KERN_ERR "%s : fsg->curlun = curlun = NULL\n", __func__);
        fsg->curlun = curlun = NULL;
        fsg->bad_lun_okay = 0;

        /* INQUIRY and REQUEST SENSE commands are explicitly allowed
         * to use unsupported LUNs; all others may not. */
        if (fsg->cmnd[0] != SC_INQUIRY &&
                fsg->cmnd[0] != SC_REQUEST_SENSE) {
            DBG(fsg, "unsupported LUN %d\n", fsg->lun_cdrom);
            return -EINVAL;
        }
    }

    /* Check that only command bytes listed in the mask are non-zero */
    fsg->cmnd[1] &= 0x1f;            /* Mask away the LUN */
    for (i = 1; i < cmnd_size; ++i) {
        if (fsg->cmnd[i] && !(mask & (1 << i))) {
            if (curlun) {
                curlun->sense_data = SS_INVALID_FIELD_IN_CDB;
            }
            DBG(fsg, "SS_INVALID_FIELD_IN_CDB\n");
            return -EINVAL;
        }
    }

    return 0;
}


static int do_scsi_command(struct fsg_dev_cdrom *fsg)
{
    struct fsg_buffhd_cdrom     *bh;
    int                         rc;
    int                         reply = -EINVAL;
    int                         i;
    static char                 unknown[16];

    dump_cdb(fsg);

    /* Wait for the next buffer to become available for data or status */
    bh = fsg->next_buffhd_to_drain = fsg->next_buffhd_to_fill;
    while (bh->state != BUF_STATE_EMPTY) {
        rc = sleep_thread(fsg);
        if (rc) {
            printk(KERN_ERR "%s sleep_thread() failed : %d\n", __func__, rc);
            return rc;
        }
    }
    fsg->phase_error = 0;
    fsg->short_packet_received = 0;

    down_read(&fsg->filesem);    /* We're using the backing file */

    switch (fsg->cmnd[0]) {

    case SC_INQUIRY:
        fsg->data_size_from_cmnd = fsg->cmnd[4];
        if ((reply = check_command(fsg, 6, DATA_DIR_TO_HOST,
                (1<<4), 0,
                "INQUIRY")) == 0) {
            reply = do_inquiry(fsg, bh);
        }
        break;

    case SC_MODE_SELECT_6:
        fsg->data_size_from_cmnd = fsg->cmnd[4];
        if ((reply = check_command(fsg, 6, DATA_DIR_FROM_HOST,
                (1<<1) | (1<<4), 0,
                "MODE SELECT(6)")) == 0) {
            reply = do_mode_select(fsg, bh);
        }
        break;

    case SC_MODE_SELECT_10:
        fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]);
        if ((reply = check_command(fsg, 10, DATA_DIR_FROM_HOST,
                (1<<1) | (3<<7), 0,
                "MODE SELECT(10)")) == 0) {
            reply = do_mode_select(fsg, bh);
        }
        break;

    case SC_MODE_SENSE_10:
        fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]);
        if ((reply = check_command(fsg, 10, DATA_DIR_TO_HOST,
                (1<<1) | (1<<2) | (3<<7), 0,
                "MODE SENSE(10)")) == 0) {
            reply = do_mode_sense(fsg, bh);
        }
        break;

    case SC_READ_6:
        i = fsg->cmnd[4];
        fsg->data_size_from_cmnd = (i == 0 ? 256 : i) << 11;
        if ((reply = check_command(fsg, 6, DATA_DIR_TO_HOST,
                (7<<1) | (1<<4), 1,
                "READ(6)")) == 0) {
            reply = do_read(fsg);
        }
        break;

    case SC_READ_10:
        fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]) << 11;
        if ((reply = check_command(fsg, 10, DATA_DIR_TO_HOST,
                (1<<1) | (0xf<<2) | (3<<7), 1,
                "READ(10)")) == 0) {
            reply = do_read(fsg);
        }
        break;

    case SC_READ_CAPACITY:
        fsg->data_size_from_cmnd = 8;
        if ((reply = check_command(fsg, 10, DATA_DIR_TO_HOST,
                (0xf<<2) | (1<<8), 1,
                "READ CAPACITY")) == 0) {
            reply = do_read_capacity(fsg, bh);
        }
        break;

    case SC_REQUEST_SENSE:
        fsg->data_size_from_cmnd = fsg->cmnd[4];
        if ((reply = check_command(fsg, 6, DATA_DIR_TO_HOST,
                (1<<4), 0,
                "REQUEST SENSE")) == 0) {
            reply = do_request_sense(fsg, bh);
        }
        break;

    case SC_TEST_UNIT_READY:
        fsg->data_size_from_cmnd = 0;
        reply = check_command(fsg, 6, DATA_DIR_NONE,
                0, 1,
                "TEST UNIT READY");

        if ( fsg->curlun == NULL ) {
	        printk(KERN_ERR "%s : fsg->curlun = NULL  g_cdromdisk = %d\n", __func__,g_cdromdisk);
        }
        if ((g_cdromdisk == SS_MEDIUM_NOT_PRESENT) && (fsg->curlun != NULL)) {
            
            
            fsg->curlun->sense_data = SS_MEDIUM_NOT_PRESENT;
        }
        break;

    case SC_READ_TOC:
        fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]);
        if ((reply = check_command(fsg, 10, DATA_DIR_TO_HOST,
                (1<<1) | (1<<6) | (3<<7), 1,
                "READ TOC")) == 0) {
            reply = do_read_toc(fsg, bh);
        }
        break;

    case SC_GET_CONFIGURATION:
        fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]);
        if ((reply = check_command(fsg, 10, DATA_DIR_TO_HOST,
                (1<<1) | (3<<2) | (3<<7), 1,
                "GET CONFIGURATION")) == 0) {
            reply = do_get_configuration(fsg, bh);
        }
        break;

    case SC_GET_EVENT_STATUS_NOTIFICATION:
        fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]);
        if ((reply = check_command(fsg, 10, DATA_DIR_TO_HOST,
                (1<<1) | (1<<4) | (3<<7), 1,
                "GET EVENT STATUS NOTIFICATION")) == 0) {
            reply = do_get_event_status_notification(fsg, bh);
        }
        break;

    case SC_READ_DISC_INFORMATION:
        fsg->data_size_from_cmnd = get_be16(&fsg->cmnd[7]);
        if ((reply = check_command(fsg, 10, DATA_DIR_TO_HOST,
                (3<<7), 1,
                "READ DISC INFORMATION")) == 0) {
            reply = do_read_disc_information(fsg, bh);
        }
        break;

    case SC_CDROM_MODE_CHANGE:
        /* CDROM MODE CHANGE command from TL-PC */
    case SC_CDROM_MODE_CHANGE_BS:
        /* CDROM MODE CHANGE command from TL-BS */
        fsg->data_size_from_cmnd = 0;
        if ((reply = check_command(fsg, 6, DATA_DIR_NONE,
                0, 1,
                "CDROM MODE CHANGE")) == 0) {
            reply = do_cdrom_mode_change(fsg);
        }
        break;

    case SC_CDROM_TIMER_CANCEL:
        /* CDROM TIMER CANCEL command */
        fsg->data_size_from_cmnd = 0;
        if ((reply = check_command(fsg, 6, DATA_DIR_NONE,
                0, 0,
                "CDROM MODE CHANGE")) == 0) {
            reply = do_cdrom_timer_cancel(fsg);
        }
        break;

    default:
        fsg->data_size_from_cmnd = 0;
        sprintf(unknown, "Unknown x%02x", fsg->cmnd[0]);
        if ((reply = check_command(fsg, fsg->cmnd_size,
                DATA_DIR_UNKNOWN, 0xff, 0, unknown)) == 0) {
            if ( fsg->curlun == NULL ) {
	            printk(KERN_ERR "%s : default fsg->curlun = NULL \n", __func__);
	        }
            else{
	            fsg->curlun->sense_data = SS_INVALID_COMMAND;
            }

            reply = -EINVAL;
        }
        break;
    }
    up_read(&fsg->filesem);

    VDBG(fsg, "reply: %d, fsg->data_size_from_cmnd: %d\n",
            reply, fsg->data_size_from_cmnd);
    if (reply == -EINTR || signal_pending(current)) {
        return -EINTR;
    }

    /* Set up the single reply buffer for finish_reply() */
    if (reply == -EINVAL) {
        reply = 0;       /* Error reply length */
    }
    if (reply >= 0 && fsg->data_dir == DATA_DIR_TO_HOST) {
        reply = min((u32) reply, fsg->data_size_from_cmnd);
        bh->inreq->length = reply;
        bh->state = BUF_STATE_FULL;
        fsg->residue -= reply;
    }               /* Otherwise it's already set */
    if (readtocflg == 1) {
        fsg->residue = 0;
        readtocflg = 0;
    }

    return 0;
}

/*-------------------------------------------------------------------------*/

static int received_cbw(struct fsg_dev_cdrom *fsg, struct fsg_buffhd_cdrom *bh)
{
    struct usb_request          *req = bh->outreq;
    struct bulk_cb_wrap_cdrom   *cbw = req->buf;

    /* Was this a real packet? */
    if (req->status) {
        return -EINVAL;
    }

    /* Is the CBW valid? */
    if (req->actual != USB_BULK_CB_WRAP_LEN ||
            cbw->Signature != __constant_cpu_to_le32(
                USB_BULK_CB_SIG)) {
        DBG(fsg, "invalid CBW: len %u sig 0x%x\n",
                req->actual,
                le32_to_cpu(cbw->Signature));
        return -EINVAL;
    }

    /* Is the CBW meaningful? */
    if (cbw->Lun >= MAX_LUNS || cbw->Flags & ~USB_BULK_IN_FLAG ||
            cbw->Length <= 0 || cbw->Length > MAX_COMMAND_SIZE) {
        DBG(fsg, "non-meaningful CBW: lun_cdrom = %u, flags = 0x%x, "
                "cmdlen %u\n",
                cbw->Lun, cbw->Flags, cbw->Length);
        return -EINVAL;
    }

    /* Save the command for later */
    fsg->cmnd_size = cbw->Length;
    memcpy(fsg->cmnd, cbw->CDB, fsg->cmnd_size);
    if (cbw->Flags & USB_BULK_IN_FLAG) {
        fsg->data_dir = DATA_DIR_TO_HOST;
    } else {
        fsg->data_dir = DATA_DIR_FROM_HOST;
    }
    fsg->data_size = le32_to_cpu(cbw->DataTransferLength);
    if (fsg->data_size == 0) {
        fsg->data_dir = DATA_DIR_NONE;
    }
    fsg->lun_cdrom = cbw->Lun;
    fsg->tag = cbw->Tag;
    return 0;
}


static int get_next_command(struct fsg_dev_cdrom *fsg)
{
    struct fsg_buffhd_cdrom     *bh;
    int                         rc = 0;

    /* Wait for the next buffer to become available */
    bh = fsg->next_buffhd_to_fill;
    while (bh->state != BUF_STATE_EMPTY) {
        rc = sleep_thread(fsg);
        if (rc) {
            printk(KERN_ERR "%s sleep_thread() failed 1 : %d\n", __func__, rc);
            usb_ep_dequeue(fsg->bulk_out, bh->outreq);
            bh->outreq_busy = 0;
            bh->state = BUF_STATE_EMPTY;
            return rc;
        }
    }

    /* Queue a request to read a Bulk-only CBW */
    set_bulk_out_req_length(fsg, bh, USB_BULK_CB_WRAP_LEN);
    start_transfer(fsg, fsg->bulk_out, bh->outreq,
            &bh->outreq_busy, &bh->state);

    /* We will drain the buffer in software, which means we
     * can reuse it for the next filling.  No need to advance
     * next_buffhd_to_fill. */

    /* Wait for the CBW to arrive */
    while (bh->state != BUF_STATE_FULL) {
        rc = sleep_thread(fsg);
        if (rc) {
            printk(KERN_ERR "%s sleep_thread() failed 2 : %d\n", __func__, rc);
            usb_ep_dequeue(fsg->bulk_out, bh->outreq);
            bh->outreq_busy = 0;
            bh->state = BUF_STATE_EMPTY;
            return rc;
        }
    }
    smp_rmb();
    rc = received_cbw(fsg, bh);
    bh->state = BUF_STATE_EMPTY;

    return rc;
}

/*-------------------------------------------------------------------------*/

static int enable_endpoint(struct fsg_dev_cdrom *fsg, struct usb_ep *ep,
        const struct usb_endpoint_descriptor *d)
{
    int    rc;

    DBG(fsg, "usb_ep_enable %s\n", ep->name);
    ep->driver_data = fsg;
    rc = usb_ep_enable(ep, d);
    if (rc) {
        ERROR(fsg, "can't enable %s, result %d\n", ep->name, rc);
    }
    return rc;
}

static int alloc_request(struct fsg_dev_cdrom *fsg, struct usb_ep *ep,
        struct usb_request **preq)
{
    *preq = usb_ep_alloc_request(ep, GFP_ATOMIC);
    if (*preq) {
        return 0;
    }
    ERROR(fsg, "can't allocate request for %s\n", ep->name);
    return -ENOMEM;
}

/*
 * Reset interface setting and re-init endpoint state (toggle etc).
 * Call with altsetting < 0 to disable the interface.  The only other
 * available altsetting is 0, which enables the interface.
 */
static int do_set_interface(struct fsg_dev_cdrom *fsg, int altsetting)
{
    int	rc = 0;
    int	i;

    if (fsg->running) {
        DBG(fsg, "reset interface\n");
    }

reset:
    /* Deallocate the requests */
    for (i = 0; i < NUM_BUFFERS; ++i) {
        struct fsg_buffhd_cdrom *bh = &fsg->buffhds[i];

        if (bh->inreq) {
            usb_ep_free_request(fsg->bulk_in, bh->inreq);
            bh->inreq = NULL;
        }
        if (bh->outreq) {
            usb_ep_free_request(fsg->bulk_out, bh->outreq);
            bh->outreq = NULL;
        }
    }

    fsg->running = 0;
    if (altsetting < 0 || rc != 0) {
        return rc;
    }

    DBG(fsg, "set interface %d\n", altsetting);

    /* Allocate the requests */
    for (i = 0; i < NUM_BUFFERS; ++i) {
        struct fsg_buffhd_cdrom	*bh = &fsg->buffhds[i];

        rc = alloc_request(fsg, fsg->bulk_in, &bh->inreq);
        if (rc != 0) {
            goto reset;
        }
        rc = alloc_request(fsg, fsg->bulk_out, &bh->outreq);
        if (rc != 0) {
            goto reset;
        }
        bh->inreq->buf = bh->outreq->buf = bh->buf;
        bh->inreq->context = bh->outreq->context = bh;
        bh->inreq->complete = bulk_in_complete;
        bh->outreq->complete = bulk_out_complete;
    }

    fsg->running = 1;

    return rc;
}

static void adjust_wake_lock(struct fsg_dev_cdrom *fsg)
{
    int ums_active = 0;
    int i;

    spin_lock_irq(&fsg->lock);

    if (fsg->config) {
        for (i = 0; i < fsg->nluns; ++i) {
            if (backing_file_is_open(&fsg->luns[i])) {
                ums_active = 1;
            }
        }
    }

    if (ums_active) {
        wake_lock(&fsg->wake_lock);
    } else {
        wake_unlock(&fsg->wake_lock);
    }

    spin_unlock_irq(&fsg->lock);
}

/*
 * Change our operational configuration.  This code must agree with the code
 * that returns config descriptors, and with interface altsetting code.
 *
 * It's also responsible for power management interactions.  Some
 * configurations might not work with our current power sources.
 * For now we just assume the gadget is always self-powered.
 */
static int do_set_config(struct fsg_dev_cdrom *fsg, u8 new_config)
{
    int	rc = 0;

    /* Disable the single interface */
    if (fsg->config != 0) {
        DBG(fsg, "reset config\n");
        fsg->config = 0;
        rc = do_set_interface(fsg, -1);
    }

    /* Enable the interface */
    if (new_config != 0) {
        fsg->config = new_config;
        if ((rc = do_set_interface(fsg, 0)) != 0) {
            fsg->config = 0;	// Reset on errors
        }
    }

    switch_set_state(&fsg->sdev, new_config);
    adjust_wake_lock(fsg);
    return rc;
}


/*-------------------------------------------------------------------------*/

static void handle_exception(struct fsg_dev_cdrom *fsg)
{
    siginfo_t                   info;
    int                         sig;
    int                         i;
    int                         num_active;
    struct fsg_buffhd_cdrom     *bh;
    enum fsg_state_cdrom        old_state;
    u8                          new_config;
    struct lun_cdrom            *curlun;
    int                         rc;

    DBG(fsg, "handle_exception state: %d\n", (int)fsg->state);
    /* Clear the existing signals.  Anything but SIGUSR1 is converted
     * into a high-priority EXIT exception. */
    for (;;) {
        sig = dequeue_signal_lock(current, &current->blocked, &info);
        if (!sig) {
            break;
        }
        if (sig != SIGUSR1) {
            if (fsg->state < FSG_STATE_EXIT) {
                DBG(fsg, "Main thread exiting on signal\n");
            }
            raise_exception(fsg, FSG_STATE_EXIT);
        }
    }

    /* Cancel all the pending transfers */
    for (i = 0; i < NUM_BUFFERS; ++i) {
        bh = &fsg->buffhds[i];
        if (bh->inreq_busy) {
            usb_ep_dequeue(fsg->bulk_in, bh->inreq);
        }
        if (bh->outreq_busy) {
            usb_ep_dequeue(fsg->bulk_out, bh->outreq);
        }
    }

    /* Wait until everything is idle */
    for (;;) {
        num_active = 0;
        for (i = 0; i < NUM_BUFFERS; ++i) {
            bh = &fsg->buffhds[i];
            num_active += bh->outreq_busy;
        }
        if (num_active == 0) {
            break;
        }
        if (sleep_thread(fsg)) {
            return;
        }
    }

    /*
    * Do NOT flush the fifo after set_interface()
    * Otherwise, it results in some data being lost
    */
    if ((fsg->state != FSG_STATE_CONFIG_CHANGE) ||
        (fsg->new_config != 1)) {
        /* Clear out the controller's fifos */
        if (fsg->bulk_in_enabled) {
            usb_ep_fifo_flush(fsg->bulk_in);
        }
        if (fsg->bulk_out_enabled) {
            usb_ep_fifo_flush(fsg->bulk_out);
        }
    }
    /* Reset the I/O buffer states and pointers, the SCSI
     * state, and the exception.  Then invoke the handler. */
    spin_lock_irq(&fsg->lock);

    for (i = 0; i < NUM_BUFFERS; ++i) {
        bh = &fsg->buffhds[i];
        bh->state = BUF_STATE_EMPTY;
    }
    fsg->next_buffhd_to_fill = fsg->next_buffhd_to_drain =
            &fsg->buffhds[0];

    new_config = fsg->new_config;
    old_state = fsg->state;

    if (old_state == FSG_STATE_ABORT_BULK_OUT)
        fsg->state = FSG_STATE_STATUS_PHASE;
    else {
        for (i = 0; i < fsg->nluns; ++i) {
            curlun = &fsg->luns[i];
            curlun->prevent_medium_removal = 0;
            curlun->sense_data = SS_NO_SENSE;
            curlun->sense_data_info = 0;
            curlun->info_valid = 0;
        }
        fsg->state = FSG_STATE_IDLE;
    }
    spin_unlock_irq(&fsg->lock);

    /* Carry out any extra actions required for the exception */
    switch (old_state) {
    default:
        break;

    case FSG_STATE_ABORT_BULK_OUT:
        DBG(fsg, "FSG_STATE_ABORT_BULK_OUT\n");
        spin_lock_irq(&fsg->lock);
        if (fsg->state == FSG_STATE_STATUS_PHASE) {
            fsg->state = FSG_STATE_IDLE;
        }
        spin_unlock_irq(&fsg->lock);
        break;

    case FSG_STATE_RESET:
        /* really not much to do here */
        break;

    case FSG_STATE_CONFIG_CHANGE:
        rc = do_set_config(fsg, new_config);
        break;

    case FSG_STATE_EXIT:
    case FSG_STATE_TERMINATED:
        do_set_config(fsg, 0);              /* Free resources */
        spin_lock_irq(&fsg->lock);
        fsg->state = FSG_STATE_TERMINATED;  /* Stop the thread */
        spin_unlock_irq(&fsg->lock);
        break;
    }
}

/*-------------------------------------------------------------------------*/

static int fsg_main_thread(void *fsg_)
{
    struct fsg_dev_cdrom    *fsg = fsg_;

    int rc = 0;


    /* Allow the thread to be killed by a signal, but set the signal mask
     * to block everything but INT, TERM, KILL, and USR1. */
    allow_signal(SIGINT);
    allow_signal(SIGTERM);
    allow_signal(SIGKILL);
    allow_signal(SIGUSR1);

    /* Allow the thread to be frozen */
    set_freezable();

    /* Arrange for userspace references to be interpreted as kernel
     * pointers.  That way we can pass a kernel pointer to a routine
     * that expects a __user pointer and it will work okay. */
    set_fs(get_ds());

    /* The main loop */
    while (fsg->state != FSG_STATE_TERMINATED) {
        if (exception_in_progress(fsg) || signal_pending(current)) {
            printk(KERN_ERR "%s: exception\n", __func__);
            handle_exception(fsg);
            continue;
        }

        if (!fsg->running) {

            rc = sleep_thread(fsg);
            if (rc) {
                printk(KERN_ERR "%s: sleep_thread is failed 1. rc=%d\n", __func__, rc);
            }

            continue;
        }

        if (g_readflg != READ_DATA_IDEL) {
            

            rc = sleep_thread(fsg);
            if (rc) {
                printk(KERN_ERR "%s: sleep_thread is failed 2. rc=%d\n", __func__, rc);
            }

            continue;
        }

        if (get_next_command(fsg)) {
            continue;
        }

        spin_lock_irq(&fsg->lock);
        if (!exception_in_progress(fsg)) {
            fsg->state = FSG_STATE_DATA_PHASE;
        }
        spin_unlock_irq(&fsg->lock);

		if (do_scsi_command(fsg) || finish_reply(fsg)) {
            continue;
        }

        spin_lock_irq(&fsg->lock);
        if (!exception_in_progress(fsg)) {
            fsg->state = FSG_STATE_STATUS_PHASE;
        }
        spin_unlock_irq(&fsg->lock);

        if (g_readflg == READ_DATA_IDEL) {
            if (send_status(fsg)) {
                continue;
            }
        }

        spin_lock_irq(&fsg->lock);
        if (!exception_in_progress(fsg)) {
            fsg->state = FSG_STATE_IDLE;
        }
        spin_unlock_irq(&fsg->lock);
    }

    spin_lock_irq(&fsg->lock);
    fsg->thread_task = NULL;
    spin_unlock_irq(&fsg->lock);

    /* Let the unbind and cleanup routines know the thread has exited */
    complete_and_exit(&fsg->thread_notifier, 0);
}


/*-------------------------------------------------------------------------*/

static ssize_t show_file(struct device *dev, struct device_attribute *attr,
        char *buf)
{
    return 0;
}

static ssize_t store_file(struct device *dev, struct device_attribute *attr,
        const char *buf, size_t count)
{
    return 0;
}


static DEVICE_ATTR(file, 0444, show_file, store_file);

/*-------------------------------------------------------------------------*/

static void fsg_release(struct kref *ref)
{
    struct fsg_dev_cdrom    *fsg = container_of(ref, struct fsg_dev_cdrom, ref);

    kfree(fsg->luns);
    kfree(fsg);
}

static void lun_release(struct device *dev)
{
    struct fsg_dev_cdrom    *fsg = dev_get_drvdata(dev);

    kref_put(&fsg->ref, fsg_release);
}


/*-------------------------------------------------------------------------*/

static int __init fsg_alloc(void)
{
    struct fsg_dev_cdrom        *fsg;

    fsg = kzalloc(sizeof *fsg, GFP_KERNEL);
    if (!fsg) {
        return -ENOMEM;
    }
    spin_lock_init(&fsg->lock);
    init_rwsem(&fsg->filesem);
    kref_init(&fsg->ref);
    init_completion(&fsg->thread_notifier);

    the_fsg = fsg;
    return 0;
}

static ssize_t print_switch_name(struct switch_dev *sdev, char *buf)
{
    return sprintf(buf, "%s\n", DRIVER_NAME);
}

static ssize_t print_switch_state(struct switch_dev *sdev, char *buf)
{
    struct fsg_dev_cdrom    *fsg = container_of(sdev, struct fsg_dev_cdrom, sdev);
    return sprintf(buf, "%s\n", (fsg->config ? "online" : "offline"));
}

static void
fsg_function_unbind(struct usb_configuration *c, struct usb_function *f)
{
    struct fsg_dev_cdrom    *fsg = func_to_dev(f);
    int                     i;
    struct lun_cdrom        *curlun;

    DBG(fsg, "fsg_function_unbind\n");
    clear_bit(REGISTERED, &fsg->atomic_bitflags);

    /* Unregister the sysfs attribute files and the LUNs */
    for (i = 0; i < fsg->nluns; ++i) {
        curlun = &fsg->luns[i];
        if (curlun->registered) {
            device_remove_file(&curlun->dev, &dev_attr_file);
            device_unregister(&curlun->dev);
            curlun->registered = 0;
        }
    }

    /* If the thread isn't already dead, tell it to exit now */
    if (fsg->state != FSG_STATE_TERMINATED) {
        raise_exception(fsg, FSG_STATE_EXIT);
        wait_for_completion(&fsg->thread_notifier);

        /* The cleanup routine waits for this completion also */
        complete(&fsg->thread_notifier);
    }

    /* Free the data buffers */
    for (i = 0; i < NUM_BUFFERS; ++i) {
        kfree(fsg->buffhds[i].buf);
    }
   switch_dev_unregister(&fsg->sdev);
}

static int
fsg_function_bind(struct usb_configuration *c, struct usb_function *f)
{
    struct usb_composite_dev *cdev = c->cdev;
    struct fsg_dev_cdrom    *fsg = func_to_dev(f);
    int            rc;
    int            i;
    int            id;
    struct lun_cdrom        *curlun;
    struct usb_ep        *ep;
    char            *pathbuf, *p;

    fsg->cdev = cdev;
    DBG(fsg, "fsg_function_bind\n");

    dev_attr_file.attr.mode = 0644;

    /* Find out how many LUNs there should be */
    i = fsg->nluns;
    if (i == 0) {
		i = 1;
    }
    if (i > MAX_LUNS) {
        ERROR(fsg, "invalid number of LUNs: %d\n", i);
        rc = -EINVAL;
        goto out;
    }

    /* Create the LUNs, open their backing files, and register the
     * LUN devices in sysfs. */
    fsg->luns = kzalloc(i * sizeof(struct lun_cdrom), GFP_KERNEL);
    if (!fsg->luns) {
        rc = -ENOMEM;
        goto out;
    }
    fsg->nluns = i;

    for (i = 0; i < fsg->nluns; ++i) {
        curlun = &fsg->luns[i];
        curlun->ro = 0;
        curlun->dev.release = lun_release;
        /* use "usb_mass_storage" platform device as parent if available */
        if (fsg->pdev) {
            curlun->dev.parent = &fsg->pdev->dev;
        } else {
            curlun->dev.parent = &cdev->gadget->dev;
        }
        dev_set_drvdata(&curlun->dev, fsg);
        dev_set_name(&curlun->dev,"lun%d", i);
        
        rc = device_register(&curlun->dev);
        if (rc != 0) {
            INFO(fsg, "failed to register LUN%d: %d\n", i, rc);
            goto out;
        }
        rc = device_create_file(&curlun->dev, &dev_attr_file);
        if (rc != 0) {
            ERROR(fsg, "device_create_file failed: %d\n", rc);
            device_unregister(&curlun->dev);
            goto out;
        }
        curlun->registered = 1;
        kref_get(&fsg->ref);
    }

    /* allocate interface ID(s) */
    id = usb_interface_id(c, f);
	if (id < 0) {
        return id;
    }

    intf_desc.bInterfaceNumber = id;

    ep = usb_ep_autoconfig(cdev->gadget, &fs_bulk_in_desc);
    if (!ep) {
        goto autoconf_fail;
    }
	ep->driver_data = fsg;      /* claim the endpoint */
	fsg->bulk_in = ep;

    ep = usb_ep_autoconfig(cdev->gadget, &fs_bulk_out_desc);
    if (!ep) {
        goto autoconf_fail;
    }
    ep->driver_data = fsg;      /* claim the endpoint */
    fsg->bulk_out = ep;

    rc = -ENOMEM;

    if (gadget_is_dualspeed(cdev->gadget)) {
        /* Assume endpoint addresses are the same for both speeds */
        hs_bulk_in_desc.bEndpointAddress =
                fs_bulk_in_desc.bEndpointAddress;
        hs_bulk_out_desc.bEndpointAddress =
                fs_bulk_out_desc.bEndpointAddress;

        f->hs_descriptors = hs_function;
    }

    /* Allocate the data buffers */
    for (i = 0; i < NUM_BUFFERS; ++i) {
        struct fsg_buffhd_cdrom    *bh = &fsg->buffhds[i];

        /* Allocate for the bulk-in endpoint.  We assume that
         * the buffer will also work with the bulk-out (and
         * interrupt-in) endpoint. */
        bh->buf = kmalloc(fsg->buf_size, GFP_KERNEL);
        if (!bh->buf) {
            goto out;
        }
        bh->next = bh + 1;
    }
    fsg->buffhds[NUM_BUFFERS - 1].next = &fsg->buffhds[0];

    fsg->thread_task = kthread_create(fsg_main_thread, fsg,
            shortname);
    if (IS_ERR(fsg->thread_task)) {
        rc = PTR_ERR(fsg->thread_task);
        ERROR(fsg, "kthread_create failed: %d\n", rc);
        goto out;
    }

    INFO(fsg, "Number of LUNs=%d\n", fsg->nluns);

    pathbuf = kmalloc(PATH_MAX, GFP_KERNEL);
    for (i = 0; i < fsg->nluns; ++i) {
        curlun = &fsg->luns[i];
        if (backing_file_is_open(curlun)) {
            p = NULL;
            if (pathbuf) {
                p = d_path(&curlun->filp->f_path,
                       pathbuf, PATH_MAX);
                if (IS_ERR(p)) {
                    p = NULL;
                }
            }
            LINFO(curlun, "ro=%d, file: %s\n",
                    curlun->ro, (p ? p : "(error)"));
        }
    }
    kfree(pathbuf);

    set_bit(REGISTERED, &fsg->atomic_bitflags);

    /* Tell the thread to start working */
    wake_up_process(fsg->thread_task);
    return 0;

autoconf_fail:
    ERROR(fsg, "unable to autoconfigure all endpoints\n");
    rc = -ENOTSUPP;

out:
    DBG(fsg, "fsg_function_bind failed: %d\n", rc);
    fsg->state = FSG_STATE_TERMINATED;    /* The thread is dead */
    fsg_function_unbind(c, f);
    return rc;
}

static int fsg_function_set_alt(struct usb_function *f,
        unsigned intf, unsigned alt)
{
    struct fsg_dev_cdrom                    *fsg = func_to_dev(f);
    struct usb_composite_dev                *cdev = fsg->cdev;
    const struct usb_endpoint_descriptor	*d;
    int                                     rc;

    DBG(fsg, "fsg_function_set_alt intf: %d alt: %d\n", intf, alt);



    
    if (g_timerflg == 0) {
        
        rc = f_timer_mod_timer();
        printk(KERN_INFO "@@@@@@ f_timer_mod_timer ret=%d @@@@@@\n", rc);
        if(rc == 0)
        {
            
            printk(KERN_DEBUG "%s: @@@@@@ f_timer_end @@@@@@\n", __func__);
            f_timer_end();
        }
        else {
            
            g_timerflg = 1;
        }
    }



    /* Enable the endpoints */
    d = ep_desc(cdev->gadget, &fs_bulk_in_desc, &hs_bulk_in_desc);
    rc = enable_endpoint(fsg, fsg->bulk_in, d);
    if (rc) {
        return rc;
    }
    fsg->bulk_in_enabled = 1;

    d = ep_desc(cdev->gadget, &fs_bulk_out_desc, &hs_bulk_out_desc);
    rc = enable_endpoint(fsg, fsg->bulk_out, d);
    if (rc) {
        usb_ep_disable(fsg->bulk_in);
        fsg->bulk_in_enabled = 0;
        return rc;
    }
    fsg->bulk_out_enabled = 1;
    fsg->bulk_out_maxpacket = le16_to_cpu(d->wMaxPacketSize);
    fsg->new_config = 1;
    raise_exception(fsg, FSG_STATE_CONFIG_CHANGE);
    return 0;
}

static void fsg_function_disable(struct usb_function *f)
{
    struct fsg_dev_cdrom    *fsg = func_to_dev(f);
    DBG(fsg, "fsg_function_disable\n");

    /* Disable the endpoints */
    if (fsg->bulk_in_enabled) {
        DBG(fsg, "usb_ep_disable %s\n", fsg->bulk_in->name);
        usb_ep_disable(fsg->bulk_in);
        fsg->bulk_in_enabled = 0;
    }
    if (fsg->bulk_out_enabled) {
        DBG(fsg, "usb_ep_disable %s\n", fsg->bulk_out->name);
        usb_ep_disable(fsg->bulk_out);
        fsg->bulk_out_enabled = 0;
    }
    fsg->new_config = 0;
    raise_exception(fsg, FSG_STATE_CONFIG_CHANGE);
}

int cdrom_open(u32 filesize)
{
    g_timerflg = 0; 
    g_filesize = filesize;
    g_cdromdisk = SS_MEDIUM_NOT_PRESENT;
    g_readflg = READ_DATA_IDEL;
    return 0;
}

int cdrom_close(void)
{

    if (!g_filesize) {
        return 0;
    }


    g_timerflg = 0; 
    g_filesize = 0;
    g_cdromdisk = SS_MEDIUM_NOT_PRESENT;
    g_readflg = READ_DATA_IDEL;
    return 0;
}

static int __init fsg_probe(struct platform_device *pdev)
{
    struct usb_mass_storage_platform_data *pdata = pdev->dev.platform_data;
    struct fsg_dev_cdrom *fsg = the_fsg;

    fsg->pdev = pdev;
    printk(KERN_INFO "fsg_probe pdata: %p\n", pdata);

    if (pdata) {
        if (pdata->vendor) {
            fsg->vendor = pdata->vendor;
        }

        if (pdata->product) {
            fsg->product = pdata->product;
        }

        if (pdata->release) {
            fsg->release = pdata->release;
        }
        fsg->nluns = pdata->nluns;
    }

    return 0;
}

static struct platform_driver fsg_platform_driver = {
    .driver = { .name = "usb_cdrom", },
    .probe = fsg_probe,
};

int cdrom_bind_config(struct usb_configuration *c)
{
    int		               rc;
    struct fsg_dev_cdrom   *fsg;

    printk(KERN_INFO "cdrom_bind_config\n");
    rc = fsg_alloc();
    if (rc) {
        return rc;
    }
    fsg = the_fsg;

    spin_lock_init(&fsg->lock);
    init_rwsem(&fsg->filesem);
    kref_init(&fsg->ref);
    init_completion(&fsg->thread_notifier);

    the_fsg->buf_size = BULK_BUFFER_SIZE;
    the_fsg->sdev.name = DRIVER_NAME;
    the_fsg->sdev.print_name = print_switch_name;
    the_fsg->sdev.print_state = print_switch_state;
    rc = switch_dev_register(&the_fsg->sdev);
    if (rc < 0) {
        goto err_switch_dev_register;
    }

    rc = platform_driver_register(&fsg_platform_driver);
    if (rc != 0) {
        goto err_platform_driver_register;
    }

    wake_lock_init(&the_fsg->wake_lock, WAKE_LOCK_SUSPEND,
               "usb_cdrom");

    fsg->cdev = c->cdev;
    fsg->function.name = shortname;
    fsg->function.descriptors = fs_function;
    fsg->function.bind = fsg_function_bind;
    fsg->function.unbind = fsg_function_unbind;
    fsg->function.setup = fsg_function_setup;
    fsg->function.set_alt = fsg_function_set_alt;
    fsg->function.disable = fsg_function_disable;

	/* start disabled */
	fsg->function.hidden = 1;


    rc = usb_add_function(c, &fsg->function);
    if (rc != 0) {
        goto err_usb_add_function;
    }

    return 0;

err_usb_add_function:
    wake_lock_destroy(&the_fsg->wake_lock);
    platform_driver_unregister(&fsg_platform_driver);
err_platform_driver_register:
    switch_dev_unregister(&the_fsg->sdev);
err_switch_dev_register:
    kref_put(&the_fsg->ref, fsg_release);

    return rc;
}

static struct android_usb_function cdrom_function = {
    .name = "usb_cdrom",
    .bind_config = cdrom_bind_config,
};

static int __init init(void)
{
    android_register_function(&cdrom_function);
    return 0;
}
module_init(init);


