/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    mmc_spi.h
 * @brief   MMC over SPI driver header.
 *
 * @addtogroup MMC_SPI
 * @{
 */

#ifndef _MMC_SPI_H_
#define _MMC_SPI_H_

#if HAL_USE_MMC_SPI || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define MMC_CMD0_RETRY              10
#define MMC_CMD1_RETRY              100
#define MMC_ACMD41_RETRY            100
#define MMC_WAIT_DATA               10000

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    MMC_SPI configuration options
 * @{
 */
/**
 * @brief   Delays insertions.
 * @details If enabled this options inserts delays into the MMC waiting
 *          routines releasing some extra CPU time for the threads with
 *          lower priority, this may slow down the driver a bit however.
 *          This option is recommended also if the SPI driver does not
 *          use a DMA channel and heavily loads the CPU.
 */
#if !defined(MMC_NICE_WAITING) || defined(__DOXYGEN__)
#define MMC_NICE_WAITING            TRUE
#endif

/**
 * @brief   Number of positive insertion queries before generating the
 *          insertion event.
 */
#if !defined(MMC_POLLING_INTERVAL) || defined(__DOXYGEN__)
#define MMC_POLLING_INTERVAL        10
#endif

/**
 * @brief   Interval, in milliseconds, between insertion queries.
 */
#if !defined(MMC_POLLING_DELAY) || defined(__DOXYGEN__)
#define MMC_POLLING_DELAY           10
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !HAL_USE_SPI || !CH_USE_EVENTS
#error "MMC_SPI driver requires HAL_USE_SPI and CH_USE_EVENTS"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  MMC_UNINIT = 0,                           /**< Not initialized.           */
  MMC_STOP = 1,                             /**< Stopped.                   */
  MMC_WAIT = 2,                             /**< Waiting card.              */
  MMC_INSERTED = 3,                         /**< Card inserted.             */
  MMC_READY = 4,                            /**< Card ready.                */
  MMC_READING = 5,                          /**< Reading.                   */
  MMC_WRITING = 6                           /**< Writing.                   */
} mmcstate_t;

/**
 * @brief   MMC/SD over SPI driver configuration structure.
 */
typedef struct {
  /**
   * @brief SPI driver associated to this MMC driver.
   */
  SPIDriver             *spip;
  /**
   * @brief SPI low speed configuration used during initialization.
   */
  const SPIConfig       *lscfg;
  /**
   * @brief SPI high speed configuration used during transfers.
   */
  const SPIConfig       *hscfg;
} MMCConfig;

/**
 * @extends MMCSDBlockDevice
 *
 * @brief   Structure representing a MMC/SD over SPI driver.
 */
typedef struct {
  /**
   * @brief Virtual Methods Table.
   */
  const struct MMCSDBlockDeviceVMT *vmt;
  /**
   * @brief Driver state.
   */
  mmcstate_t            state;
  /**
   * @brief Current configuration data.
   */
  const MMCConfig       *config;
  /**
   * @brief Card insertion event source.
   */
  EventSource           inserted_event;
  /**
   * @brief Card removal event source.
   */
  EventSource           removed_event;
  /**
   * @brief MMC insertion polling timer.
   */
  VirtualTimer          vt;
  /**
   * @brief Insertion counter.
   */
  uint_fast8_t          cnt;
  /***
   * @brief Addresses use blocks instead of bytes.
   */
  bool_t                block_addresses;
  /**
   * @brief Card CID.
   */
  uint32_t                  cid[4];
  /**
   * @brief Card CSD.
   */
  uint32_t                  csd[4];
  /**
   * @brief Total number of blocks in card.
   */
  uint32_t              capacity;
} MMCDriver;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   Returns the driver state.
 *
 * @param[in] mmcp      pointer to the @p MMCDriver object
 * @return              The driver state.
 *
 * @api
 */
#define mmcGetDriverState(mmcp) ((mmcp)->state)

/**
 * @brief   Returns the write protect status.
 *
 * @param[in] mmcp      pointer to the @p MMCDriver object
 * @return              The card state.
 * @retval FALSE        card not inserted.
 * @retval TRUE         card inserted.
 *
 * @api
 */
#define mmcIsWriteProtected(mmcp) mmc_lld_is_write_protected(mmcp)
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void mmcInit(void);
  void mmcObjectInit(MMCDriver *mmcp);
  void mmcStart(MMCDriver *mmcp, const MMCConfig *config);
  void mmcStop(MMCDriver *mmcp);
  bool_t mmcConnect(MMCDriver *mmcp);
  bool_t mmcDisconnect(MMCDriver *mmcp);
  bool_t mmcStartSequentialRead(MMCDriver *mmcp, uint32_t startblk);
  bool_t mmcSequentialRead(MMCDriver *mmcp, uint8_t *buffer);
  bool_t mmcStopSequentialRead(MMCDriver *mmcp);
  bool_t mmcStartSequentialWrite(MMCDriver *mmcp, uint32_t startblk);
  bool_t mmcSequentialWrite(MMCDriver *mmcp, const uint8_t *buffer);
  bool_t mmcStopSequentialWrite(MMCDriver *mmcp);
  bool_t mmcSync(MMCDriver *mmcp);
  bool_t mmcGetInfo(MMCDriver *mmcp, BlockDeviceInfo *bdip);
  bool_t mmcErase(MMCDriver *mmcp, uint32_t startblk, uint32_t endblk);
  bool_t mmc_lld_is_card_inserted(MMCDriver *mmcp);
  bool_t mmc_lld_is_write_protected(MMCDriver *mmcp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_MMC_SPI */

#endif /* _MMC_SPI_H_ */

/** @} */
