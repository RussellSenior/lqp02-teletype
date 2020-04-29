This is a project motivated by a desire to create a daisywheel teletype, to give a
similar experience to that of a Model 33ASR teletype, but with upper and lowercase
characters. Thanks to Hugh Pyle and his fantastic [Twitch channel](https://www.twitch.tv/33asr) for the
inspiration.  Thanks also to Chris Gregg and his project:
https://github.com/tofergregg/IBM-Wheelwriter-Hack and to this very similar project
by Stephen Casner: https://github.com/IBM-1620/Junior.

The goal is to intercept user input from the keyboard and send it to a remote computer
and receive the echoed characters from the remote computer and send the corresponding
commands to the printing mechanism for hardcopy output.

I obtained an IBM Wheelwriter 5 from the Tektronix Surplus Store (open two days a month,
first and third Thursdays, from 2pm to 4pm) on the Beaverton Oregon campus of Tektronix.
This typewriter came with the PC Printer option, which hangs on the back of the case
and interfaces by a 6 (or 7?) pin bus. The cost of the typewriter was $25. I purchased
two new-in-box ribbon cartridges from the same location for $0.50 each.

In the process of working on the IBM Wheelwriter, I was given a couple of DEC LQP02 serial
daisywheel printers. This repository reflects the work on the LQP02.  These have some advantages,
including the availability of a native RS-232 interface and full ASCII character set wheels.

This project uses a Teensy 3.1 with a PS/2 keyboard, and two serial ports.  One serial port
talks to the printer and the other talks to a remote computer.  It decodes keyboard events using
the Ps2KeyboardHost library from Steve Benz: https://github.com/SteveBenz/PS2KeyboardHost,
sends them to the remote computer, reads data coming from the remote computer and sends them
to the printer's serial port.

The printer is constrained in the rate at which it can turn bytes into print, so flow control
is important. It turns out that the printer does XON/XOFF flow control, and the remote computer
does RTS/CTS hardware flow control, so the Teensy program looks for XON/XOFF from the printer
and stops reading from the computer serial port when it last saw XOFF, which lets the hardware
UART's flow control just work.

One of the disadvantages of the LQP02 is that it was intended as a strictly output device, and
as a result being able to see what you are typing is not prioritized.  I saw a video on youtube
(https://www.youtube.com/watch?v=TRxz4x45i54) which used the vertical motion control to roll
the platen to lift the active printing line up above the printing mechanism at appropriate
moments so that you can see what you have typed.  On the LQP02, 12 lines seems about right.
When rolling the paper back down to continue printing, I am rolling an extra line down and then
back up one to hopefully improve vertical registration.  Thanks Blake Thomas for the idea.

The other disadvantage is that the LQP02 prints bi-directionally, which means that it needs
to pause on input to see if more input is coming. If there is, then it can effectively read
ahead and see the opportunity to print backwards. This introduces an annoying lag between input
and when the characters are struck. So far, no means of turning off this delay has been found.

There are a bunch of TODO's:

 * Add a toggle switch to switch between Line and Local mode;
 * Support the mini-DIN8 connected Sun Keyboard;
 * Support output to an IBM Wheelwriter;
