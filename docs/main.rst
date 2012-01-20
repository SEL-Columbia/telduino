Meter Reporting

The meter will report every N seconds a single serial text string
for each of M meters to the sheeva plug over a USB-Serial connection.

(To facilitate integration, the sheeva could run a USB-Serial to HTTP daemon
that makes the serial text strings appear like HTTP posts)

Each of these M reports will contain the

#) real energy (Watts)
#) apparent energy (Volt-Amps)
#) maximum voltage (Volts)
#) maximum amperage (Amps)
#) relay state (Open/Closed)
#) circuit_id (integer 1-20)
#) sequence number (count since start)
#) error flags?



Testing list

#) does meter retain continuity of readings after power outage?
#)

todo - how do we handle the three circuit types (main, consumer, shed)?
todo - how do we handle non-responsive meter?
