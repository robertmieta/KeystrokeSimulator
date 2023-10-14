Keystroke Simulator
===================

A minimal program that keeps your applications online and computer from going to sleep.

Overview
========

- Run **Keystroke Simulator.exe**
- The application is accessible via the system tray
- Designed to run passively in the background and use as minimal CPU as possible
- Goes to sleep while you're active, and starts working while you're inactive

Run Automatically on Startup
============================

Follow these steps to have it always run on computer startup

- Right-click and create a shortcut of **Keystroke Simulator.exe**
- Press the **Windows Key** + **R**
- Type **shell:startup**
- Copy the shortcut to that folder

Notes
=====

- For Windows only
- Due to the size limitations of GetLastInputInfo() using an unsigned 32 bit integer, you will need to shut down or restart your computer at least once every 49 days to make sure it reports the correct value and doesn't overflow.