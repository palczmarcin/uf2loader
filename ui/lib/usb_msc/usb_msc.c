/*
 * usb_msc.c
 * USB Mass Storage Class implementation
 * Naprawione dla TinyUSB w pico-sdk 2.1.0
 */

#include <string.h>
#include "tusb.h"
#include "usb_msc.h"
#include "text_directory_ui.h"
#include "sdmmc.h"

bool sd_card_inserted(void);

static uint16_t msc_block_size = 512;

void usb_msc_init(void)
{
    // Nowe API TinyUSB dla pico-sdk 2.x
    tusb_init();
}

static bool is_mounted = false;

void tud_mount_cb(void)   { is_mounted = true;  }
void tud_umount_cb(void)  { is_mounted = false; }

uint8_t tud_msc_get_maxlun_cb(void) { return 0; }

void tud_msc_inquiry_cb(uint8_t lun, uint8_t p_vendor_id[8],
                         uint8_t p_product_id[16], uint8_t p_product_rev[4])
{
    (void)lun;
    static const char vendor[8]    = "PICO";
    static const char product[16]  = "UF2LOADER_MSC";
    static const char revision[4]  = "1.0 ";
    memcpy(p_vendor_id,   vendor,   sizeof(vendor));
    memcpy(p_product_id,  product,  sizeof(product));
    memcpy(p_product_rev, revision, sizeof(revision));
}

void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
{
    (void)lun;
    unsigned int count = MMC_get_sector_count();
    if (count > 0) {
        *block_size  = msc_block_size;
        *block_count = count;
    } else {
        *block_size  = 0;
        *block_count = 0;
    }
}

bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition,
                             bool start, bool load_eject)
{
    (void)lun; (void)power_condition; (void)start; (void)load_eject;
    return true;
}

int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset,
                            void *buffer, uint32_t bufsize)
{
    (void)lun; (void)offset;
    if (!MMC_disk_read(buffer, lba, bufsize / msc_block_size)) return -1;
    return (int32_t)bufsize;
}

bool tud_msc_is_writable_cb(uint8_t lun) { (void)lun; return true; }

int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset,
                             uint8_t *buffer, uint32_t bufsize)
{
    (void)lun; (void)offset;
    return MMC_disk_write(buffer, lba, bufsize / msc_block_size)
           ? (int32_t)bufsize : -1;
}

void tud_msc_write10_flush_cb(uint8_t lun) { (void)lun; }

bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    bool inserted = sd_card_inserted();
    if (!inserted) {
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3A, 0x00);
        return false;
    }
    return true;
}

int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16],
                          void *buffer, uint16_t bufsize)
{
    (void)buffer; (void)bufsize;
    switch (scsi_cmd[0]) {
        default:
            tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
            return -1;
    }
}

bool usb_msc_is_mounted(void) { return is_mounted && tud_ready(); }
void usb_msc_stop(void)       { tud_disconnect(); }
