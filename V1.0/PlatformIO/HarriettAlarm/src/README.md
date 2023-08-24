-- An STM32 BlackPill based alarm clock.

Called the Harriett clock because that's the name of my personal trainer.
Sometimes I forget to set the alarm on my phone or otherwise botch getting up.
This is embarrassing, rude and annoying :-(
So I invented the Harriett alarm so that it was obvious even to an old short-sighted
person like me what is happening the next day:

  * Picture of me tucked up in bed => I get to sleep in. No alarm set.
  * Picture of me with a phone to my ear => Work! Alarm!
  * Picture of me lifting weights with Harriett counting the reps(*) => Training day! Alarm!

(*) We always disagree on the number of reps I've done.

With added special features like
 * you can set the time (!)
 * each alarm can have a different tone and volume (!!)

Originally this was built to use a DFRobot MP3 player, and there was a selection of 
eight alarm tones to choose from. However the module consumes so much power at start-up
(and takes so long to start) that it was causing problems. So I replaced it with a simple
set of beeping tones using a Timer and PWM. The code (and the hardware design) for the MP3 
player is still in place however, should you wish to actually build this thing.

The rest of the hardware involves a Waveshare epaper screen, some buttons
and a 3D printed box.

In order to help with debugging I also embedded my Forth virtual machine, which communicates
over a serial port. This was surprisingly useful and not at all silly and done for the sake of it.
Honest.