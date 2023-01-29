# Embedded Systems Plotter (ESPlot) - Computer-Software v0.9

## Connecting to a microcontroller

1. Connect your device to your computer.
2. Start the OpenGLPlotter.exe.
3. Press *Open Connection* <img src="Icons\connect.ico" title="Open Connection" height="18"/>  or use the shortcut Ctrl+O.
4. Select your USB or Serial COM Device from the dropdown menu. If you use a Serial COM device, you have to provide the correct Baudrate.
5. Confirm your selection with OK. You will be notified if there are any errors. (See *Errors connecting to your device*)

The connection should now be established. A control panel as well as your predefined plots are now visible on screen.

## Start transmitting and receiving data

1. Set the Time(ms) value to adjust how often you want to transmit and receive the signals. This value should be equal to or lower than *PROCESS_FREQUENCY*.
2. Press *Start receiving signals...* <img src="Icons\Play.ico" title="Start receiving signals..." height="18"/> to start transmitting and receiving signals.
3. The most recently received signals can be seen in the *Signals received* table.
4. To change the value of a transmit signal, choose it in the *Variable list* and set it to the value of your choice. The value can be input as decimal, hexadecimal or binary. Commit to the value by pressing *Send*. If you checked *Send on ENTER* the value will be saved on enter. Your new value will now be transmit the next time you transmit and receive signals.

While you are sending and receiving signals you can also use the *Command string*. This is a terminal and will send a maximum of 16 characters to your microcontroller.

## Record and export data

1. Select the signals you want to record by having a check mark next to it in the *Signals received* section. (Standard: All signals selected)
2. Press *Start recording signals...*<img src="Icons\Record.ico" title="Start recording signals..." height="18"/>. The record time is now shown next to the record button.
3. When you are finished recording your signals, press *Stop receiving signals* <img src="Icons\Stop.ico" title="Stop receiving signals..." height="18"/>.
4. To export your recorded data as a .mat press *Export data*. Each signal is saved as an individual vector.

## Creating plots

If you chose not to define your plots on your microcontroller or want to add another one you can do so with the buttons beneath the *Plotter windows*.
There you can also delete or rename the selected plot.

### Creating a signal plot

Signal plots are used to plot any number of signals over time.

1. Beneath *Plotter windows* press <img src="Icons\Add.ico" title="Add Plot" height="18"/> *Add Plot*.
2. Enter the name of your new plot.
3. In the dropdown menu choose *Signal plot*.

Your empty plot will now open automatically with no signals allocated to it. To add signals to the plot do as follows:

1. Select your plot in *Plotter windwos*.
2. Select the signal you want to allocate to the plot.
3. Beneath *Signals received* press <img src="Icons\Add.ico" title="Add Signal" height="18"/> *Add Signal*.
5. Confirm your chosen signal.
6. Adjust the line width of the graph.
7. Choose the color of your graph.

To delete a signal from a signal plot select the plot and the signal and press <img src="Icons\Remove.ico" title="Remove Signal" height="18"/> *Remove Signal*.

ESPlot gives you the option to configure your signals regarding signal plots. If you select a signal and press <img src="Icons\Sig_Settings.ico" title="Signal Settings" height="18"/> *Signal Settings* you will receive an overview with all the signal-plots the signal is allocated to.
Here you can manage line-width and color of the signal in each plot.

### Creating a X-Y-plot

In X-Y-plots you have to choose at least two signals, one for each axis. As the number of values gets larger over time, it is advisable to lower the *Past value brightness*.

1. Beneath *Plotter windows* press <img src="Icons\Add.ico" title="Add Plot" height="18"/> *Add Plot*.
2. Enter the name of your new plot.
3. In the dropdown menu choose *X-Y plot*.

Your empty plot will now open automatically with no signals allocated to it. To add graphs to the plot do as follows:

1. Select your plot in *Plotter windwos*.
2. Beneath *Signals received* press <img src="Icons\AddXY.ico" title="Add X-Y" height="18"/> *Add X-Y*.
3. Select the signal to be allocated to the X-axis.
4. Select the signal to be allocated to the Y-axis.
5. Confirm your chosen signals.
6. Adjust the line width of the graph.
7. Choose the color of your graph.

To add multiple graphs, repeat the steps above.

If you want to remove a graph entirely from a plot, select the plot in which to remove a graph and press <img src="Icons\RemoveXY.ico" title="Remove X-Y" height="18"/> *Remove X-Y*. In the new window select the graph to be removed and confirm your selection.

## Adjusting Plots

If you configured at least one plot in your program, it will open after connecting to your microcontroller. Plots you create also open automatically.
Alternatively you can open your plot by double clicking on it in the *Plotter windows* segment.
The plots currently opened are shown with a green outline, the closed plots with a red one.

Above the plot you have several options to configure the view as well as the trigger in signal plots.

* Export data: Export your plot as a .bmp or .jpg.
* <img src="Icons\grid.ico" title="Grid" height="18"/> Grid: Enable or disable the grid. (Ctrl+G)
* <img src="Icons\yTicks.ico" title="Y-Ticks" height="18"/> Y-Ticks: Enable or disable the labeling of the Y-axis.
* <img src="Icons\xTicks.ico" title="X-Ticks" height="18"/> X-Ticks: Enable or disable the labeling of the X-axis.
* <img src="Icons\title.ico" title="Title" height="18"/> Title: Enable or disable the on-plot title.
* <img src="Icons\autoscale.ico" title="Auto-scaling" height="18"/> Auto-scaling: Enable or disable auto-scaling. This will overwrite your minimum and maximum values to show every value in the active view.
* <img src="Icons\triggerRise.ico" title="Trigger rising" height="18"/> Trigger rising: Enable or disable to trigger on the rising edge.
* <img src="Icons\triggerFall.ico" title="Trigger falling" height="18"/> Trigger falling: Enable or disable to trigger on the falling edge.
* <img src="Icons\triggerLeft.ico" title="Move trigger left" height="18"/> Move trigger left: Move the trigger position to the left.
* <img src="Icons\triggerRight.ico" title="Move trigger right" height="18"/> Move trigger right: Move the trigger position to the right.
* <img src="Icons\triggerReset.ico" title="Reset trigger position" height="18"/> Reset trigger position: Resets the position of the trigger to its standard.
* <img src="Icons\zoomXY.ico" title="Zoom XY" height="18"/> Zoom XY: Zoom in on the selected area.
* <img src="Icons\zoomY.ico" title="Zoom Y" height="18"/> Zoom Y: Zoom in on the selected area whithout changing the X-axis.
* <img src="Icons\zoomX.ico" title="Zoom X" height="18"/> Zoom X: Zoom in on the selected area whithout changing the Y-axis.
* <img src="Icons\panXY.ico" title="Pan" height="18"/> Pan: While zoomed, pan the active view.
* <img src="Icons\legend.ico" title="Legend" height="18"/> Legend: Enable or disable the legend of the plot.
* <img src="Icons\Stats.ico" title="Statistics" height="18"/> Statistics: Enable or disable the statistics of the plot.
* <img src="Icons\resetStats.ico" title="Reset Statistics" height="18"/> Reset statistics: Delete the statistics history.
* Close: Close the plot window.

Beneath the plot you have several options to manually adjust the shown values.

* X-maximum value: Adjust the maximum value of the X-axis. Only available in X-Y-plots.
* X-minimum value: Adjust the minimum value of the X-axis. Only available in X-Y-plots.
* Y maximum value: Adjust the maximum value of the Y-axis.
* Y minimum value: Adjust the minimum value of the Y-axis.
* Past value brightness: Adjust if and how fast old values should become transparent.
* Y-grid-step: Add or remove grid lines on the Y-axis.
* X-grid-step: Add or remove grid lines on the X-axis.
* Autoscale-centering: Add additional space above and beneath the extremes while auto-scaling.
* Number of samples: Adjust the number of samples shown in the plot. This will change the time shown. Only available in signal-plots
* Trigger value: Adjust the value to activate the trigger.

When your legend is shown you can change the appearence of your signals or graphs by clicking on the signal. You can either have only points, connected points or have your signal or graph not visible.

If you have many plots and want to clean up your screen, go to the main ESPlot windows and press <img src="Icons\windows.ico" title="Organize plot windows" height="18"/> *Organize plot windows*. The plots are now positioned switching between the top left and the top right corner of your screen.

## Manage preferences

Starting ESPlot all settings from previous uses are reset. But there is a way to save your preferences regarding windows, font and colors.
In the *Options* tab at the top of the window you can find different options regarding preferences. There you can edit your preferences, load preferences and choose from up to four predefined styles.
Preferences are only available when connected to a microcontroller.
In the tab *Windows and Grid* you can adjust the window size as well as the standard grid when creating a plot.
The other options and the other tabs contain options concerning colors, font, font size etc. .
At the bottom of the window you can see an example of the cosmetic changes you made.

# Errors

## Errors connecting to your device

### Error during connecting to the device

This error occurs if connecting to your device is not possible. There are a number of reasons for this, for example:

* The device is not connected.
* You are using the wrong driver (only possible using Windows).
* Your serial chip is already in use.

Other sources are also possible.

### Error during parsing the information frame

After the connection is established this error can occur. The error can either be a faulty connection between your serial chip and your microcontroller or be based on your software for the following reasons:

* Your microcontroller is not sending any signals.
* Your microcontroller is sending signals with a wrong frequency. Remember to send data according to the *PROCESS_FREQUENCY* in *comm_prot_config.h*.
* Other settings of your communications protocol differ, for example Baudrate, MSB first, etc. .