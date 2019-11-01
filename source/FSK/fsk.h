/*******************************************************************************
  @file     FSK.c
  @brief    FSK Driver
  @author   Grupo 1 Laboratorio de Microprocesadores
 ******************************************************************************/

#ifndef _FSK_H_
#define _FSK_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define FSK_TX_QUEUE_SIZE 16
#define FSK_RX_QUEUE_SIZE 16

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize FSK driver
*/
void fskInit ();

/**
 * @brief Check if a new byte was received
 * @return A new byte has been received
*/
bool fskIsRxMsg();

/**
 * @brief Check how many bytes were received
 * @return Quantity of received bytes
*/
size_t fskGetRxMsgLength();

/**
 * @brief Read a received message. Non-Blocking
 * @param msg Buffer to paste the received bytes
 * @param cant Desired quantity of bytes to be pasted
 * @return Real quantity of pasted bytes
*/
size_t fskReadMsg(uint8_t * msg, size_t cant);

/**
 * @brief Write a message to be transmitted. Non-Blocking
 * @param msg Buffer with the bytes to be transferred
 * @param cant Desired quantity of bytes to be transferred
 * @return Real quantity of bytes to be transferred
*/
size_t fskWriteMsg(const uint8_t* msg, size_t cant);

/**
 * @brief Check if all bytes were transfered
 * @return All bytes were transfered
*/
bool fskIsTxMsgComplete();

/*******************************************************************************
 ******************************************************************************/

#endif // _FSK_H_
