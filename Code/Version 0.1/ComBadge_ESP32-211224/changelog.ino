/*
      Last Edit: 17/01/2025:

      TOUCH ERROR (222) RESTARTS WHEN WAKE FROM TOUCH
      TOUCH WORKS ONCE THEN NOT AGAIN
      
      Lowered Button Tone as >1xxx was too high
      
      /!\ reduced ESP32 Arduino Core down to version 2.0.17 as version >3.x does not compile /!\

      HARDWARE UPDATE:
      swapped 600mA power converter to 1.2A

      --------------------------------------------------------------------------------------
      Last Edit: 26/03/2024 -
      Updated Setup_AP (combadgeap) to same non-lcars version as on Scicorder mini
      Updated deffinitions etc
      added cert.h
      added new dac control for sounds
            
      --------------------------------------------------------------------------------------
      
      Last Edit: 25/03/2024 -
      Updated sleep control to include the example layout to trigger sleep and wake without errors
      /!\ Brown-Out caused at wake by sounds when powered by FTDI..

      --------------------------------------------------------------------------------------
      
      Last Edit: 12/03/2024 -
      Updated TouchPIn deffinitions to easier to read version, modified pin assignment based on changes to PCB v0.1a.
      Marked wake_from_lid_open as "only needed for Scicorder mini but left in for debug".
      Updated about page by removing changelog and putting into seperate file
      Updated PCB to include TP4050x LiPo Charger + USB-C charging-only connector
      Added Labels in loop

      --------------------------------------------------------------------------------------
      
      Last Edit: 11/11/2023  - Updated wake timer functions based on updated esp32 core..
      
 */
