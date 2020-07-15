# EENG18020 Ultrasonic lab
# Week 1 - Characterisation:

# 1.Intro:
{
8sections:
7X3 = 21
3or4X8 = 24or32
instuctions
}
measure distance use the time-of-flight techniques:

1. send out an ultrasonic signal
2. signal reflected back by the surface of any object
3. a receiver will receive the reflected signal
4. micro-controller calculates the time difference between the sent and received signals(time-of=flight)

By knowing speed of sound and time-of-flight:

distance travelled = (speed of wave propagation) X (time-of-flight)

using ultrasonic transducers transmit and receive ultrasonic signals.
bulid circuit to: generate, amplify, sent and receive these signals, and program a micro-controller to control the transmitter and process the recived signal in order to measure the distance.

PC -> CPU (CC1350 Micro-Controller) -> PWM -> Amplifer -> Ultrasonic Transmitter --> Ultrasound --> Obstacle -->Ultrasound Receiver --> Amplifer -> ADC --> PC

# 2. Characterising Ultrasonic Transducer:

Ultrasonic transducer is a device that converts between electrical energy and kinetic energy(sound).
It receive an input signal and only produce a output at a certain frequency, known as "fundamental frequency".
Feed the transducers with a frequency close enough to its fundamental frequency to produce a significant output.

First task:
characteris the given transducers and find their fundamental frequencies.

Plot a diagram of the frequency response and find the fundamental frequency.


# 3. Charaterising the second Transducer
two transducers
Task 2:
onr for send and one for receive
to make they are matched.
make sure any difference in the fundamental frequencies btween the two.


# 4. Charaterising the combined system with both transducers

System will be utilising two transducers.
input signal will be attenuated by both transducers from input to output.

Task 3:
Multiply the obtained frequency response together to get a cascaded frequency response,plot it on a graph, interpolate to find the resonant frequency

connect the transmitting transducer to the signal generator, connect the receiving transducer to the oscilloscope.

Place a reflective object at a relatively close distance (30cm) in front of both transducers, so the ultrasonic signal bounces off and arrives at the receiver.

Calculate the gain of system as the output voltage divided by the input.

Task 4: Use signal generator to produce a sine wave, with amplitude 10Vpp, use a frequency sweep to determine the frequency response of the combined system. Plot the
frequency response and determine the resonant frequency. How close is it to the cascaded
approximation?

Task 5: Increase and decrease the distance between the object and the transducers, determine the change in the frequency response. Based on the change in the frequency response,
what is the nature of the air when modelled as an electrical component (resistor, inductor,
capacitor, resistor-capacitor, etc.)?

# Week 2 - Amplifier

# 1.Intro

build two amplifiers to amplify the signal from the transducers built last section.
for a 10Vpp Transmitter input -> receiver output voltage only 300mVpp
micro-controller = input signal and output signal of the entire system has to be 3.3 Vpp
therefore, utilise amplifiers to increase the strength of the signal for both transmission and reception.


# 2.Non-inverting Amplifier

to amplify a 3.3Vpp signal to 10Vpp, gain will be approxiately 3.
TL071 operational amplifiers will be used.


# 3.Transmitter Amplifier

build a non-inverting amplifier with gain being 3, it amplify a 3.3Vpp square wave to 10Vpp.
positive supply to +15V
negative supply to -15V
signal generator to simulate the input signal from controller
frequency = 40kHz
waveform = square
duty cycle = 52%
amplitude = 3.3Vpp
DC offset = 1.65V


Task 1: Based on equation, make choice of Rf and Rg, build the circuit as shown in Figure 3, measure your output signal to make sure the amplifier is working as expected.


# 4.Receiver Amplifier

The receiver amplifier is slightly different. 
ultrasound signal received by the receiver will be a sine wave with 0V offset.
amplitude depends on the distance travelled.
required output from receiving amplifier need to be within 0.1-3.3V
if obtained 330mVpp:
	amplification rate = 10
	because receiving amplifier 3.3Vpp amplitude.

Task 2: 

choose amplification rate for system, make choice of Rf and Rg
Build circuits as shown in Figure 4, measure output signal.

Note circuit is exactly the same as the two you have built.

Task 3:

Connect both transducers and amplifiers, connect signal generator to the transmitter amplifier and generate a 3.3Vpp square wave, measure the output from receiver amplifier, find the minimal and maximum distance range that your system can handle.

Note down system's minimal detect distance, when build the entire system, never put any object closer to this distance, could be dangerous.


# 5. Noise Reduction

if significant noise occur, might come from power supply.
reduce the noise from power supply and amplifier by placing capacitors around them.
shown in Figure 6.
if decide to put capacitors
be careful with the capacitor values and the polarity.

long leg = positive sign



# Week 3 - ADC

1.Intro

Test the Analogue to Digital Converter (ADC) on the CC1350 micro-controller board
import projects adc_ss adc_cont and tirtos_CC1350_xxxxxx into
Code Composer Studio in PC


# 2.Sampling
