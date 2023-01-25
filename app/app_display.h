/**
 * @file    app_display.h
 * @author  Jovanny Noé Casillas Franco
 * @brief   Header file of Display.
 *
 * The Display header file is to make the structures and we use in differents files, also, to declarate the functions prototypes
 * that we are using in the source file.
 * 
 * @note    We need take in count that we have the extern with the declaration of structures because we used MISRA.
 * 
 */
#ifndef APP_DISPLAY_H__
#define APP_DISPLAY_H__

extern void Display_Init( void );
extern void Display_Task( void );

#endif