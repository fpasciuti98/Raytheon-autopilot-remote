Write and read Raytheon Seatalk data packages directly from the Seatalk data port.
The aim of the project was to create a remote control to remotly operate a Raytheon marine autopilot. 
From the remote the user can modify and see the current vessel's heading that the autopilot is mantaining.
The code runs on the remote receiver's ATmega32 powered by the autopilot power line and connected to the Seatalk data port. 
Serial data signal need to be converted from ATmega 0-5V to Seatalk 12-0V.
