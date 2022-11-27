<h1> # Amplified_Fencer </h1>

Code + MaxMsp Patches for Hardware used for Amplfied Fencer. Created with the support of V2_ Lab for Unstable Media, Rotterdam during the 2022 Summer Sessions Programme.

![afbeelding](https://user-images.githubusercontent.com/24521356/202808169-dcfe59c8-2ca7-49db-9c58-a28d7fbf0500.png)


** text retrieved from https://v2.nl **

Amplified Fencer is a research project by Gökay Atabek that investigates observational practices and implicit composition with the help of experimental interfaces. It is presented as a series of performance pieces.

There is undeniably an inherent choreography in any sport, emerging out of the battle between the opponents. The title Amplified Fencer refers to the way the work makes this choreography visible and audible. In the piece, a fencer’s movements are dissected via motion and orientation sensors, reformatted with the help of a neural network and remapped to an experimental audio synthesizer. This allows the fencer’s set of moves to be translated into soundscapes and syncopated rhythm snippets. The fencer constructs a composition by performing a realistic, logical set of moves; the fencer (now a musician) is forced to play the instrument implicitly, not intentionally. This process is prone to the same failures that would lose the fencer points in a match; the neural network is only trained to recognize absolutely perfect positions. Anything short of that will result in garbled output.

Fencing is a sport that requires a high degree of precision and good proprioception. A fencer’s focus and stress response can usually be perceived in the nuances of their moves, though these are difficult for the untrained eye to interpret. To make this world visible, the fencer is wired with an EEG device that actively monitors brain activity. This data is parsed, mapped to a video synthesis engine and projected onto the playing area.

With this computer-enabled amplification, the human-versus-human dialogue of a fencing match seemingly shifts to human-versus-machine. But there is no human-versus-machine dialogue: at best, it is a human-versus-designer/engineer dialogue. Does this axiom still hold true with the involvement of AI in this relationship?

** text retrieved from https://v2.nl **

<h2> Tech Stuff </h2>

Amplified Fencer Movement 1 uses an Arduino Nano sewn onto a standart fencing glove and a <a href= "https://choosemuse.com/muse-s/"> Muse S Headband </a> for transnmitting movement and biomedical data from the fencer. 

<h3> The Glove </h3>
The Arduino in question is a new (at the time of this writing) 3..3v board called <a href="https://docs.arduino.cc/hardware/nano-33-iot"> Nano IoT 33 </a>. This board was selected for its built in 6 DOF IMU and Wifi module. To expand the boards scope a digital compass module based on the <a href="https://www.farnell.com/datasheets/1683374.pdf"> QMC5883L 3 Axis MAgentometer </a> was glued to the board. The Adafruit version of the module is discontinued but there are alternatives made by no-name Chinese manufacturers. The one I got looks like this: 


![afbeelding](https://user-images.githubusercontent.com/24521356/202803504-c5c872eb-6463-4b75-b7a0-a3b1a100dd7d.png)

I will skip the schematics as QMC5883L is an I2C device, and schematics would vary depending on the board you choose to use. Please refer to the datasheet of the microcontroller board of your choosing for its I2C bus connections. 

The library I have used for the QMC5883L is included under <i> Dependencies </i>. I am certain there are better alternatives but for now this works. DO NOT FORGET TO CALIBRATE WITH THE PROVIDED .ino FILE !

The glove sends all the sensor readings as OSC to whatever destination you prefer. Feel free to change the adresses at your discretion. To learn more about OSC and why MIDI 2.0 is utterly useless visit https://ccrma.stanford.edu/groups/osc/index.html

Personally I prefer not doing any data formatting on the microcontroller itself as aanytime a change is necessary that requires me to flash the board again, which is (almost always) way more work than scaling at the recieving end. 

<h3> The Headband </h3>

The video engine used in Amplified Fencer is controlled by data transmitted via the Muse S headband. Cool stuff!

![afbeelding](https://user-images.githubusercontent.com/24521356/202806358-bcd40f2c-4526-4583-b345-141fb6abd429.png)

This of course comes with its own subset of problems. First of all, no manufacturer of consumer-grade EEG devices seems to last more than 5 years. Then again Muse is the exception; I think they have been around for a whopping 8 years.

The Muse S is their newest gizmo and it seems to be the least fragile one, which is always a good thing. The software is closed open-source. By that I mean the Muse SDK is not available for download anymore. Additionaly the headband refuses to connect to PC and Mac (haven't tested Linux) and insists on a mobile OS. However the amazing guys at Mind Monitor have relased an app that lets you connect your Muse headband to your phone and use your phone to send OSC to your PC. Nifty! Get it here https://mind-monitor.com

You can use the MuseStarter.maxpat to recieve OSC from the Mind Monitor app. It listen to the default port of the Mind Monitor app but you can cahgne this with sending a message to the 1st inlet. 

The outputs are all the data Mind Monitor reads from the Muse headband. They are all labeled with the ranges listed, just hover over the outlets!

Keep in mind that this is in no way shape or form a scientific device; but its not snake oil either. I recommend taking some baseline readings and then taking some more readings while doing "something" My favourite activities were playing Tetris or unfolding 3D shapes in my head. The readings are genuine with some margin of error.

To use the MuseStarter.maxpat you need the <a href="https://cycling74.com/articles/cnmat-odot-tools-for-osc-and-beyond"> odot </a> package from CNMAT. It is available in Max Package Manager and also in the <i> Dependencies </i> folder.

<h3> Machine Learning Magic </h3>

An important part of the performance is an applied machine learning system for making sounds. This comes courtesy of <a href="https://www.federicovisi.com"> Federico Visi </a> and his super cool Max Package called GiMLeT available at https://github.com/federicoVisi/GIMLeT.

I use a heavily modified version of one of the example insturmetns "hacked" to send MIDI to Ableton however it is embarrassingly clunky and buggy so it will be posted once cleaned.

Also check the ml* package made by Benjamin Day Smith, which the GIMLeT is based on https://www.benjamindaysmith.com/ml-machine-learning-toolkit-in-max.

<h3> Thanks </h3>

I would like to thank V2 Lab fo Unstable Media for making this project possible. Beyond the entity I would like to Sieta for her warm hosting and spot on email game, Sebastian for voluntering to be a target and his immaculate compressor skills, Richard for his enthusiam in bringing out the subwoofers, Florian for curatorial heavy-lifting, Steffan and Alex for all the beers and finally Fai for controlling the piece "behind the desk". I would also like to thank fellow resident Francesco Luzzana for all extra inspiration during the residency.

Finally I would like to thank you, the reader for ignoring all of my typos.


