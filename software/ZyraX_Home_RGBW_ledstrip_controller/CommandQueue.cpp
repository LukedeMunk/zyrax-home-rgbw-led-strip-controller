/******************************************************************************/
/*
 * File:    CommandQueue.cpp
 * Author:  Luke de Munk
 * Version: 0.9.0
 * 
 * Brief:   Command queue with some additional functions.
 * 
 *          More information:
 *          https://github.com/LukedeMunk/zyrax-home-rgbw-led-strip-controller
 */
/******************************************************************************/
#include "CommandQueue.h"

/******************************************************************************/
/*!
  @brief    Constructor.
*/
/******************************************************************************/
CommandQueue::CommandQueue() {
    _l.setTag("CommandQueue");
    _queuePointer = 0;
    _queueIsEmpty = true;
}

/******************************************************************************/
/*!
  @brief    Returns command for the specified index.
  @param    index               Index of the command in the queue (default: 0)
  @returns  Command             Command in the queue
*/
/******************************************************************************/
Command CommandQueue::getCommand(uint8_t index) {
    return _commandQueue[index];
}

/******************************************************************************/
/*!
  @brief    Returns true if queue is empty.
  @returns  bool                True if queue is empty
*/
/******************************************************************************/
bool CommandQueue::isEmpty() {
    return _queueIsEmpty;
}

/******************************************************************************/
/*!
  @brief    Appends command to queue.
  @param    command             Command object
  @returns  bool                True if success
*/
/******************************************************************************/
bool CommandQueue::pushCommand(Command command) {
    if (_queuePointer >= MAX_QUEUE_ITEMS) {
        _l.loge("Command queue full, cannot save command");
        return false;
    }

    if (command.priority == PRIORITY_LOW) {
        _pushCommand(command, _queuePointer);
    } else {
        /* Check where command need to be placed based on priority */
        for (uint8_t i = 0; i < MAX_QUEUE_ITEMS; i++) {
            if (_commandQueue[i].priority < command.priority) {
                _pushCommand(command, i);
                break;
            }
        }
    }

    _queuePointer++;
    _queueIsEmpty = false;

    _l.logi("Command added to command queue. Number of commands to be executed: " + String(_queuePointer));

   _cleanUp();
    
    return true;
}

/******************************************************************************/
/*!
  @brief    Pops command with the specified index in queue.
  @param    index               Index in command queue
*/
/******************************************************************************/
void CommandQueue::popCommand(uint8_t index) {
    if (_queueIsEmpty) {
        _l.logw("Queue is empty, no items popped");
        return;
    }

    /* Shift items one to the left */
    for (uint8_t i = index+1; i < MAX_QUEUE_ITEMS; i++) {
        _commandQueue[i-1] = _commandQueue[i];
    }
    
    _l.logi("Command deleted from command queue");

    _queuePointer--;
    
    if (_queuePointer == 0) {
        _queueIsEmpty = true;
        _l.logi("No commands left in queue");
    } else {
        _l.logi("Number of commands to be executed [" + String(_queuePointer) + "]");
    }
}

/******************************************************************************/
/*!
  @brief    Appends command to queue.
  @param    command             Command object
  @param    index               Index in command queue
  @returns  bool                True if success
*/
/******************************************************************************/
bool CommandQueue::_pushCommand(Command command, uint8_t index) {
    /* Shift queue items one to the right */
    for (uint8_t i = MAX_QUEUE_ITEMS-1; i > index; i--) {
        _commandQueue[i] = _commandQueue[i-1];
    }
    
    _commandQueue[index].command = command.command;
    _commandQueue[index].priority = command.priority;
    _commandQueue[index].parameter1 = command.parameter1;
    _commandQueue[index].parameter2 = command.parameter2;
    _commandQueue[index].parameter3 = command.parameter3;
    _commandQueue[_queuePointer].parameterString1 = command.parameterString1;

    return true;
}

/******************************************************************************/
/*!
  @brief    Cleans up redundant commands, like duplicate commands.
*/
/******************************************************************************/
void CommandQueue::_cleanUp() {
    uint8_t itemsRemoved = 0;
    
    for (uint8_t c1 = 0; c1 < _queuePointer; c1++) {
        for (uint8_t c2 = c1+1; c2 < _queuePointer; c2++) {                     //c2 = c1+1 because that is the next item to compare
            if (_commandQueue[c1].command == _commandQueue[c2].command) {
                if (_commandQueue[c1].command != 255) {
                    popCommand(c1);                                             //Pop c1 because it is overwritten by c2
                    itemsRemoved++;
                }
            }
        }
    }

    if (itemsRemoved > 0) {
        _l.logi("[" + String(itemsRemoved) + "] duplicate items removed from command queue during cleanup");
    }
    
    if (_queuePointer < itemsRemoved) {
        _l.loge("More items removed than there are in the queue");
        return;
    }
}
