#ifndef HARRIETT_BUTTONS_H
#define HARRIETT_BUTTONS_H

#include "Button.h"

typedef void (*EventHandler)(void);
/**
 * Encapsulates the two by two buttons and raises appropriate events when they are pressed
 **/
class Buttons {

public:
    Buttons(uint32_t b1, uint32_t b2, uint32_t b3, uint32_t b4)
        : _b1(Button(b1))
        , _b2(Button(b2))
        , _b3(Button(b3))
        , _b4(Button(b4))
    {
    }

    bool active()
    {
        return _active;
    }

    void update(uint32_t millis)
    {
        _b1.update(millis);
        _b2.update(millis);
        _b3.update(millis);
        _b4.update(millis);
    }

    void dispatchEvents()
    {
        _active = 0;
        // let's figure out what was clicked
        if (_b1.isActive() || _b2.isActive() || _b3.isActive() || _b4.isActive()) {
            // something's happening
            // set the flag so the shell can inhibit sleeping for a while
            _active = 1;
        }

        if (_b1.isClicked() && _b2.isNotActive()) {
            return raiseEvent(_b1ClickHandler);
        }

        if (_b2.isClicked() && _b1.isNotActive()) {
            return raiseEvent(_b2ClickHandler);
        }

        if (_b1.isLongClicked() && _b2.isNotActive()) {
            return raiseEvent(_b1LongClickHandler);
        }

        if (_b2.isLongClicked() && _b1.isNotActive()) {
            return raiseEvent(_b2LongClickHandler);
        }

        if (_b1.isLongClicked() && _b2.isPressed()) {
            return raiseEvent(_b1b2LongClickHandler);
        }

        if (_b2.isLongClicked() && _b1.isPressed()) {
            return raiseEvent(_b1b2LongClickHandler);
        }

        if (_b1.isLongPressed() || _b1.isRepeating()) {
            return raiseEvent(_b1RepeatClickHandler);
        }

        if (_b2.isLongPressed() || _b2.isRepeating()) {
            return raiseEvent(_b2RepeatClickHandler);
        }

        if (_b3.isClicked() && _b4.isNotActive()) {
            return raiseEvent(_b3ClickHandler);
        }

        if (_b4.isClicked() && _b3.isNotActive()) {
            return raiseEvent(_b4ClickHandler);
        }

        if (_b3.isLongClicked() && _b4.isNotActive()) {
            return raiseEvent(_b3LongClickHandler);
        }

        if (_b4.isLongClicked() && _b3.isNotActive()) {
            return raiseEvent(_b4LongClickHandler);
        }

        if (_b3.isLongClicked() && _b4.isPressed()) {
            return raiseEvent(_b3b4LongClickHandler);
        }

        if (_b4.isLongClicked() && _b3.isPressed()) {
            return raiseEvent(_b3b4LongClickHandler);
        }

        if (_b3.isLongPressed() || _b3.isRepeating()) {
            return raiseEvent(_b3RepeatClickHandler);
        }

        if (_b4.isLongPressed() || _b4.isRepeating()) {
            return raiseEvent(_b4RepeatClickHandler);
        }

        handled();
    }

    void handled()
    {
        _b1.handled();
        _b2.handled();
        _b3.handled();
        _b4.handled();
    }

    void reset()
    {
        _b1.reset();
        _b2.reset();
        _b3.reset();
        _b4.reset();
    }

    void setB1ClickHandler(EventHandler h)
    {
        _b1ClickHandler = h;
    }

    void setB2ClickHandler(EventHandler h)
    {
        _b2ClickHandler = h;
    }

    void setB3ClickHandler(EventHandler h)
    {
        _b3ClickHandler = h;
    }

    void setB4ClickHandler(EventHandler h)
    {
        _b4ClickHandler = h;
    }

    void setB1LongClickHandler(EventHandler h)
    {
        _b1LongClickHandler = h;
    }

    void setB2LongClickHandler(EventHandler h)
    {
        _b2LongClickHandler = h;
    }

    void setB3LongClickHandler(EventHandler h)
    {
        _b3LongClickHandler = h;
    }

    void setB4LongClickHandler(EventHandler h)
    {
        _b4LongClickHandler = h;
    }

    void setB1RepeatClickHandler(EventHandler h)
    {
        _b1RepeatClickHandler = h;
    }

    void setB2RepeatClickHandler(EventHandler h)
    {
        _b2RepeatClickHandler = h;
    }

    void setB3RepeatClickHandler(EventHandler h)
    {
        _b3RepeatClickHandler = h;
    }

    void setB4RepeatClickHandler(EventHandler h)
    {
        _b4RepeatClickHandler = h;
    }

    void setB1B2LongClickHandler(EventHandler h)
    {
        _b1b2LongClickHandler = h;
    }

    void setB3B4LongClickHandler(EventHandler h)
    {
        _b3b4LongClickHandler = h;
    }

protected:
    Button _b1;
    Button _b2;
    Button _b3;
    Button _b4;

    void raiseEvent(EventHandler h)
    {
        if (h != NULL) {
            h();
        }
        handled();
    }

    bool _active = 0;
    uint8_t _down = 0;

    EventHandler _b1ClickHandler = NULL;
    EventHandler _b2ClickHandler = NULL;
    EventHandler _b3ClickHandler = NULL;
    EventHandler _b4ClickHandler = NULL;

    EventHandler _b1RepeatClickHandler = NULL;
    EventHandler _b2RepeatClickHandler = NULL;
    EventHandler _b3RepeatClickHandler = NULL;
    EventHandler _b4RepeatClickHandler = NULL;

    EventHandler _b1LongClickHandler = NULL;
    EventHandler _b2LongClickHandler = NULL;
    EventHandler _b3LongClickHandler = NULL;
    EventHandler _b4LongClickHandler = NULL;

    EventHandler _b1b2LongClickHandler = NULL;
    EventHandler _b3b4LongClickHandler = NULL;
};
#endif