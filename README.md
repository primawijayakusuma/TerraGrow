# TerraGrow
TerraGrow is an open-source platform for real-time soil-moisture, soil-pH, temperature, and relative-humidity monitoring with automated irrigation via a Sugeno-type fuzzy controller running on ESP32. Telemetry and control are exposed through Blynk (virtual pins) for remote supervision and manual override.

TerraGrow Configuration 
TerraGrow is an open-source platform for real-time soil-moisture, soil-pH, temperature, and relative-humidity monitoring with automated irrigation via a Sugeno-type fuzzy controller running on ESP32. Telemetry and control are exposed through Blynk (virtual pins) for remote supervision and manual override.
Platform: ESP32 (NodeMCU form factor) • Sensors: Copper resistive soil-moisture probe, Aluminum soil-pH electrodes via CD4051 gating, DHT11 (temp/RH) • Actuation: 5 V relay → 5 V submersible pump • IoT: Blynk (virtual pins) • Control: Sugeno-type fuzzy policy (OFF/MID/ON)
________________________________________
0. Quick facts
•	ADC: 10-bit (0–1023).
•	Default sampling cadence: moisture ~5 s; pH ~15 s; RH/Temp ~10 s.
•	DMS gating:
o	DMSpin_soil = GPIO12 LOW for ~2 s while reading moisture; HIGH otherwise.
o	DMSpin = GPIO13 LOW for ~5 s while reading pH; HIGH otherwise.
•	Relay logic: Active-LOW (GPIO5): LOW = ON, HIGH = OFF.
•	Fuzzy output: OFF (=0), MID (=0.5 → 5 s pulse), ON (=1).
•	Blynk virtual pins: V1 pH, V3 moisture %, V4 RH, V5 temperature, V6 manual override, V7 pump state, V8 status text, V9 auto/manual. (Optional debug: V0/V2 raw ADC.)
•	Safety caps (recommend): Max ON per cycle, Min rest interval, Daily cap (time or liters).
•	Cabling: pump 18 AWG silicone; sensors 22 AWG stranded; pH pair 24 AWG shielded twisted pair; jumpers 26–28 AWG.
________________________________________
1. System overview & power tree
1.1 Blocks
•	Sensing:
o	Soil moisture (resistive copper electrodes, two-wire divider to ADC).
o	Soil pH (aluminum electrodes, high-impedance ADC path; channel gated by CD4051; shielded cable).
o	DHT11 (ambient temperature & RH).
•	Processing: ESP32; runs acquisition, calibration, fuzzy inference, and IoT telemetry.
•	Actuation: Relay module (5 V coil) drives 5 V submersible pump → 1/4″ PVC hose, check-valve recommended.
•	IoT/UX: Blynk dashboard (history charts, indicators, manual override & mode switch).
1.2 Typical power budget (indicative)
•	ESP32: 80–240 mA (peaks on Wi-Fi TX).
•	Relay coil (5 V): ~60–90 mA (module-dependent).
•	DHT11: ~2–3 mA when sampling.
•	Pump (5 V): 0.5–1.5 A depending on model & head.
•	Supply guidance:
o	If using USB 5 V: choose ≥2 A adapter; relay + pump must be powered from a rail that can truly deliver surge.
o	If using battery: include protection, 5 V regulator with headroom; fuse the pump branch.
________________________________________
2. Wiring & pin map
2.1 ESP32 pin assignments (as used in firmware)
Signal	ESP32 pin	Direction	Notes
DMSpin (pH DMS)	GPIO13	DO	Control gate/CD4051 for pH path; LOW to enable read window, then HIGH.
DMSpin_soil	GPIO12	DO	Control gate/CD4051 for moisture path; LOW to enable read window, then HIGH.
pH analog in	GPIO34 (ADC)	AI	High-impedance input; use shielded twisted pair; optional RC low-pass.
Soil-moisture analog in	GPIO35 (ADC)	AI	From copper probe divider; inverse relation (wetter → lower ADC).
DHT11 data	GPIO14	DIO	Add required pull-up as per DHT library.
Relay IN	GPIO5	DO	Active-LOW (LOW = ON); keep OFF (HIGH) at boot.
Indicator LED	GPIO2	DO	Optional heartbeat/measurement indicator.
Share GND among ESP32, sensors, and relay. Keep pump current loop physically away from analog wiring; cross at 90° if necessary.
2.2 CD4051 (if address lines used)
•	For single-channel gating, address lines can be fixed to select Y0 and only toggle INH/EN (or power) via DMSpin*.
•	If you expand channels, route A/B/C (address) to spare GPIOs; reference the CD4051 truth table (Y0–Y7).
________________________________________
3. Analog front-ends & signal conditioning
3.1 Soil-moisture (resistive copper)
•	Method: Two-wire copper electrodes form a variable resistance proportional to pore-water conductivity. Use a divider to map resistance to ADC:
o	Example: Rref = 10 kΩ from 3.3 V → node → probe → GND (or probe above Rref, either way).
o	Node → GPIO35 (ADC).
•	Polarization control: Gate power/measurement using DMSpin_soil; keep measurement short (≈2 s) to reduce electrolysis.
•	Optional RC: 10–47 kΩ series + 100 nF to ground near ADC to reduce high-frequency noise (check settling time).
Expected behavior: As moisture increases, solution conductivity rises, effective probe resistance falls, divider output goes lower, and the ADC code decreases (inverse trend).
3.2 Soil-pH (aluminum electrodes)
•	Method: Pair of aluminum electrodes in soil slurry; measure an analog potential related to ionic activity.
•	High-impedance path: Keep wiring short; use shielded twisted pair; ground shield at one end only (enclosure side) to avoid loops.
•	Gating: Pull DMSpin LOW for ~5 s to allow stabilization; then read GPIO34.
•	Optional buffer (recommended for longer runs): A rail-to-rail op-amp (e.g., MCP6002) as a unity-gain buffer before ADC; add RC (e.g., 10 kΩ + 100 nF).
Firmware mapping: a linear equation pH = a·ADC + b calibrated with two buffers (e.g., pH 4.01 & 7.00). Store a, b in config.
3.3 DHT11 (Temp/RH)
•	Data on GPIO14 with pull-up (per library).
•	Don’t poll excessively (≥1–2 s apart).
•	Cross-check with a reference (HTC-2 or similar) to compute optional one-point offsets.
________________________________________
4. Cable & connector specification
•	Pump: 18 AWG silicone, red (+) / black (−), short run; sleeve/heat-shrink at strain points.
•	Sensor loom: 22 AWG stranded multi-core from enclosure to plot; label each conductor and add keyed connectors.
•	pH pair: 24 AWG shielded twisted pair; drain/shield to analog GND at controller end only.
•	Internal jumpers: 26–28 AWG Dupont (F-M / M-M / F-F).
•	Harness protection: cable glands (M12), nylon ties, and heat-shrink assortment; waterproof junction box (IP65) for any field splice.
Connector kit: JST-XH 2.54 mm housings + crimps for sensors; 3.5 mm screw terminals for pump/relay.
________________________________________
5. Firmware configuration
5.1 Core parameters
•	ADC resolution: 10-bit.
•	Delays for DMS: soil 2 s, pH 5 s (settling windows).
•	MID duration: 5 s (relay ON pulse).
•	Sampling order (loop): moisture → pH → DHT11 → fuzzy → actuation → publish.
5.2 Virtual pins (Blynk)
Signal	Virtual pin	Update period	Widget	Units
Soil moisture %	V3	5 s	Time-series chart	%
Temperature	V5	10 s	Time-series chart	°C
Relative humidity	V4	10 s	Time-series chart	%
Soil pH	V1	15 s	Time-series chart	pH
Pump state	V7	on change	Indicator	—
Auto/Manual	V9	on change	Switch	—
Manual override	V6	on press	Button (momentary)	—
(debug) ADC pH	V0	15 s	Time-series chart	codes
(debug) ADC soil	V2	5 s	Time-series chart	codes
(status) text	V8	on change	Label	—
Never publish SSID/PASS, Template ID, or token publicly. Keep them in a local config file or private env vars.
5.3 Example platformio.ini (optional)
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
build_flags = -DCORE_DEBUG_LEVEL=0
lib_deps =
  blynkkk/Blynk@^1.3.2
  adafruit/DHT sensor library@^1.4.6
________________________________________
6. Calibration & engineering units
6.1 Soil moisture (%)
•	Two-point field calibration:
1.	Dry medium: record ADC_dry.
2.	Field-wet (near saturation, drained): record ADC_wet.
•	Build a mapping table (or simple two-point line) used by piecewise/linear interpolation in firmware.
Checks: Trend must be inverse: wetter → lower ADC. Re-do when medium/fertilizer changes.
6.2 Soil pH
•	Prepare two buffer solutions (e.g., 4.01, 7.00).
•	For each: set DMSpin LOW, wait 5 s, average multiple ADC readings → compute (a, b) for pH = a·ADC + b.
•	Verify with a third buffer (e.g., 10.00) or known agronomic solution.
6.3 Temperature & RH
•	Cross-check with a reference; compute optional offsets. Since fuzzy sets are coarse (LOW/MED/HIGH), small constant offsets are acceptable.
________________________________________
7. Fuzzy controller configuration (Sugeno)
7.1 Linguistic variables (typical bands)
•	Soil moisture [%]: LOW (≤50), MED (60–80), HIGH (≥90) with trapezoid transitions.
•	Temperature [°C]: COLD (≤18), GOOD (18–36), HOT (≥36).
•	RH [%]: DRY (≤50), NORMAL (50–60), MOIST (≥60–80+).
(Bands can be tailored to crop and locale; keep overlap for smooth transitions.)
7.2 Output & policy
•	Outputs: OFF (0), MID (0.5), ON (1).
•	Decision: Weighted average of rule consequents (Sugeno).
•	MID pulse: ~5 s ON then OFF.
•	Safety caps (firmware vars):
o	MAX_ON_PER_CYCLE (s),
o	MIN_REST_INTERVAL (s),
o	DAILY_ON_CAP (s or liters estimated).
________________________________________
8. Blynk dashboard layout (suggested)
•	Charts (time series): moisture %, pH, temperature, RH.
•	Indicators: pump state (ON/OFF), status text (linguistics).
•	Controls: Auto/Manual switch; Manual override button (momentary).
•	Alarms: push notifications on low moisture, out-of-range pH, extended pump ON.
________________________________________
9. Bring-up & functional checks (bench)
1.	Serial at 115200: verify boot logs and Wi-Fi/Blynk connect.
2.	Moisture path: with DMSpin_soil LOW, damp cloth on probe → ADC drops, % rises.
3.	pH path: with DMSpin LOW, electrode in buffer → pH reading stabilizes near buffer value.
4.	DHT11: temperature/RH reasonable for lab.
5.	Relay: press Manual override → pump ON, indicator changes; release → OFF.
6.	Auto mode: flip to AUTO and confirm decisions (OFF/MID/ON) change with induced conditions.
________________________________________
10. Enclosure & mechanical/fluidic
•	PLA enclosure with vent slots; electronics above waterline.
•	Cable glands (M12), waterproof junction box for field splices.
•	Hose: 1/4″ PVC, check-valve inline to avoid backflow; mini hose clamps.
•	Pump: ensure priming; add intake mesh/strainer; mount to reduce vibration.
________________________________________
11. Safety & reliability
•	Keep AC/mains supplies away from wet areas; if using mains adapters, use RCD/GFCI.
•	Fuse the pump rail; add reverse-polarity and transient protection as needed.
•	Separate analog harness from pump wiring; twist sensor pairs; use shield for pH.
•	Strain relief and heat-shrink all exits; never leave bare copper.
________________________________________
12. Network configuration
•	Use 2.4 GHz SSID; avoid DFS channels; moderate TX power.
•	Dedicated lab router/AP recommended for commissioning (short DHCP leases, no captive portal).
•	If using self-hosted Blynk server, ensure stable DNS/port and time sync (NTP).
________________________________________
13. Data logging & retention
•	Blynk history charts for quick look; for deeper analysis, stream serial logs to CSV.
•	Record calibration constants and firmware version alongside datasets.
•	Suggested sampling: 5–15 s for commissioning; relax for long-term deployments to save power/data.
________________________________________
14. Maintenance & re-calibration
•	Moisture: redo two-point mapping when soil medium or fertilizer regime changes.
•	pH: periodic buffer checks; update (a, b) if drift is observed (electrode aging).
•	Enclosure: inspect for condensation; re-tighten glands; check corrosion on terminals.
•	Pump: clean strainer; verify check-valve function; listen for cavitation.
________________________________________
15. Troubleshooting (quick reference)
Symptom	Likely cause	Fix
Moisture % increases but ADC rises (wrong trend)	Divider wired upside or wrong reference	Swap probe/Rref position or fix code mapping
pH reading noisy/jumpy	Long unshielded leads; no settling delay	Use shielded twisted pair; ensure DMSpin LOW ≥5 s; add RC or buffer op-amp
Relay chatters	Floating control input; fuzzy oscillation near boundary	Ensure pull-up/down; add hysteresis or minimum ON/OFF hold times
Pump ON but no flow	Not primed; check-valve stuck; supply sag	Prime line; inspect valve; check supply current capability
		


