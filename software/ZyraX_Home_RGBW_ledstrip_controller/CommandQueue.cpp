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
    _mutex = NULL;
}

/******************************************************************************/
/*!
  @brief    Initializes the queue synchronization primitive.
  @returns  bool                True when the queue can be used safely
*/
/******************************************************************************/
bool CommandQueue::initialize() {
    if (_mutex != NULL) {
        return true;
    }

    _mutex = xSemaphoreCreateMutex();
    if (_mutex == NULL) {
        _l.loge("Could not create command queue mutex");
        return false;
    }
    return true;
}

/******************************************************************************/
/*!
  @brief    Returns command for the specified index.
  @param    index               Index of the command in the queue (default: 0)
  @returns  Command             Command in the queue
*/
/******************************************************************************/
Command CommandQueue::getCommand(uint8_t index) {
    Command command;
    if (_mutex == NULL || xSemaphoreTake(_mutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE) {
        _l.loge("Could not lock command queue");
        return command;
    }
    if (index < _queuePointer) {
        command = _commandQueue[index];
    }
    xSemaphoreGive(_mutex);
    return command;
}

/******************************************************************************/
/*!
  @brief    Returns true if queue is empty.
  @returns  bool                True if queue is empty
*/
/******************************************************************************/
bool CommandQueue::isEmpty() {
    if (_mutex == NULL || xSemaphoreTake(_mutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE) {
        return true;
    }
    bool isEmpty = _queueIsEmpty;
    xSemaphoreGive(_mutex);
    return isEmpty;
}

/******************************************************************************/
/*!
  @brief    Appends command to queue.
  @param    command             Command object
  @returns  bool                True if success
*/
/******************************************************************************/
bool CommandQueue::pushCommand(Command command) {
    if (_mutex == NULL || xSemaphoreTake(_mutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE) {
        _l.loge("Could not lock command queue");
        return false;
    }

    if (_queuePointer >= MAX_QUEUE_ITEMS) {
        xSemaphoreGive(_mutex);
        _l.loge("Command queue full, cannot save command");
        return false;
    }

    if (command.priority == PRIORITY_LOW) {
        _pushCommand(command, _queuePointer);
    } else {
        bool isInserted = false;
        /* Check where command need to be placed based on priority */
        for (uint8_t i = 0; i < _queuePointer; i++) {
            if (_commandQueue[i].priority < command.priority) {
                _pushCommand(command, i);
                isInserted = true;
                break;
            }
        }
        if (!isInserted) {
            _pushCommand(command, _queuePointer);
        }
    }

    _queuePointer++;
    _queueIsEmpty = false;

    _cleanUp();
    uint8_t numberOfCommands = _queuePointer;
    xSemaphoreGive(_mutex);
    _l.logi("Command added to command queue. Number of commands to be executed: " + String(numberOfCommands));
    return true;
}

/******************************************************************************/
/*!
  @brief    Pops command with the specified index in queue.
  @param    index               Index in command queue
*/
/******************************************************************************/
void CommandQueue::popCommand(uint8_t index) {
    if (_mutex == NULL || xSemaphoreTake(_mutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE) {
        _l.loge("Could not lock command queue");
        return;
    }
    if (_queueIsEmpty) {
        _l.logw("Queue is empty, no items popped");
        xSemaphoreGive(_mutex);
        return;
    }

    if (index >= _queuePointer) {
        _l.logw("Command queue index out of range");
        xSemaphoreGive(_mutex);
        return;
    }

    _popCommand(index);
    xSemaphoreGive(_mutex);
}

/******************************************************************************/
/*!
  @brief    Atomically retrieves and removes the first queued command.
  @param    command             Destination for the retrieved command
  @returns  bool                True when a command was retrieved
*/
/******************************************************************************/
bool CommandQueue::pullCommand(Command &command) {
    if (_mutex == NULL || xSemaphoreTake(_mutex, pdMS_TO_TICKS(MUTEX_TIMEOUT_MS)) != pdTRUE) {
        _l.loge("Could not lock command queue");
        return false;
    }
    if (_queueIsEmpty) {
        xSemaphoreGive(_mutex);
        return false;
    }

    command = _commandQueue[0];
    for (uint8_t i = 1; i < _queuePointer; i++) {
        _commandQueue[i - 1] = _commandQueue[i];
    }
    _queuePointer--;
    _queueIsEmpty = (_queuePointer == 0);
    xSemaphoreGive(_mutex);
    return true;
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
    _commandQueue[index].startMode = command.startMode;
    _commandQueue[index].parameterString1 = command.parameterString1;
    _commandQueue[index].modeParameters = command.modeParameters;

    return true;
}

/******************************************************************************/
/*!
  @brief    Removes a command while the caller owns the queue mutex.
  @param    index               Index in command queue
*/
/******************************************************************************/
void CommandQueue::_popCommand(uint8_t index) {
    for (uint8_t i = index + 1; i < _queuePointer; i++) {
        _commandQueue[i - 1] = _commandQueue[i];
    }

    _queuePointer--;
    _queueIsEmpty = (_queuePointer == 0);
}

/******************************************************************************/
/*!
  @brief    Cleans up redundant commands, like duplicate commands.
*/
/******************************************************************************/
void CommandQueue::_cleanUp() {
    uint8_t c1 = 0;
    while (c1 < _queuePointer) {
        bool isRemoved = false;
        for (uint8_t c2 = c1+1; c2 < _queuePointer; c2++) {                     //c2 = c1+1 because that is the next item to compare
            bool isSameCommand = _commandQueue[c1].command == _commandQueue[c2].command;
            bool isSameConfiguration = _commandQueue[c1].command != COMMAND_CONFIGURE_MODE ||
                                       _commandQueue[c1].parameter1 == _commandQueue[c2].parameter1;
            if (isSameCommand && isSameConfiguration && _commandQueue[c1].command != 255) {
                _popCommand(c1);                                                //Caller already owns the queue mutex
                isRemoved = true;
                break;
            }
        }
        if (!isRemoved) {
            c1++;
        }
    }
}
