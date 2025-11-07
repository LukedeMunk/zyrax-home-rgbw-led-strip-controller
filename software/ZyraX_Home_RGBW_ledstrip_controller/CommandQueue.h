/******************************************************************************/
/*
 * File:    CommandQueue.h
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   Command queue with some additional functions.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#ifndef COMMANDQUEUE_H
#define COMMANDQUEUE_H
#include "Arduino.h"                                                            //For additional Arduino framework functionality, like String object
#include "Configuration.h"                                                      //For configuration variables and global constants
#include "Logger.h"                                                             //For printing and saving logs

#define PRIORITY_LOW                    0
#define PRIORITY_MEDIUM                 1
#define PRIORITY_HIGH                   2

#define MAX_QUEUE_ITEMS                 10

#define COMMAND_SET_POWER               0
#define COMMAND_SET_BRIGHTNESS          1
#define COMMAND_SET_MODE                2
#define COMMAND_DOOR_CHANGE             3

struct Command {
    uint8_t command;
    uint8_t priority = PRIORITY_LOW;
    uint16_t parameter1 = 0;
    uint16_t parameter2 = 0;
    uint16_t parameter3 = 0;
    String parameterString1;
};

class CommandQueue {
  public:
    CommandQueue();
    
    /* Direct functions */
    Command getCommand(uint8_t index = 0);
    bool isEmpty();
    bool pushCommand(Command command);
    void popCommand(uint8_t index = 0);

  private:
    bool _pushCommand(Command command, uint8_t index);
    void _cleanUp();

    Command _commandQueue[MAX_QUEUE_ITEMS];
    uint8_t _queuePointer;
    bool _queueIsEmpty;
    Logger _l;
};
#endif