# Tulip Creative Computer - Tulip Creative Computer - a portable programmable device for music, graphics, code and writing

![Tulip](https://raw.githubusercontent.com/shorepine/tulipcc/main/docs/pics/flat_tiles.jpg)

Welcome to the Tulip Creative Computer (Tulip CC)!
 
Tulip is a low power and afforable self-contained portable computer, with a touchscreen display and sound. It's fully programmable - you write code to define your music, games or anything else you can think of. It boots instantaneously into a Python prompt with a lot of built in support for music synthesis, fast graphics and text, hardware MIDI, network access and external sensors. Dive right into making something without distractions or complications. 

The entire system is dedicated to your code, the display and sound, running in real time, on specialized hardware. The hardware and software are fully open source and anyone can [buy one](https://www.makerfabs.com/the-tulip-creative-computer.html) or [build one](docs/build.md). You can use Tulip to make music, code, art, games, or just write. 

 * [**Get a Tulip from our friends at Makerfabs for only US$59**](https://www.makerfabs.com/the-tulip-creative-computer.html)
 * [**Just got a Tulip CC? Check out our getting started guide!**](docs/getting_started.md)
 * [**Want to make music with your Tulip? Check out our music tutorial here**](docs/music.md)
 * [**See the full Tulip API**](docs/tulip_api.md)
 * [**Build your own Tulip**](docs/tulip_build.md)
 * **[You can read more about the "why" or "how" of Tulip on my website!](https://notes.variogram.com/2023/02/14/tulip/)** 

 [![shore pine sound systems discord](https://raw.githubusercontent.com/shorepine/tulipcc/main/docs/pics/shorepine100.png) **Chat about Tulip on our Discord!**](https://discord.gg/TzBFkUb8pG)

[![Floyd Steinberg's Tulip Video](https://img.youtube.com/vi/1lYFjQp7Xrw/0.jpg)](https://www.youtube.com/watch?v=1lYFjQp7Xrw)

Tulip is available both as an [off the shelf or DIY hardware project (Tulip CC)](https://www.makerfabs.com/the-tulip-creative-computer.html) and a [desktop simulator app (Tulip Desktop.)](docs/tulip_desktop.md) They both have all the same features. I use Tulip Desktop while developing Tulip, but use the hardware Tulip CC when making my own personal things! If you're nervous about getting or building the hardware, [download or build the desktop version!](docs/tulip_desktop.md)

[![Tulip Desktop](https://raw.githubusercontent.com/shorepine/tulipcc/main/docs/pics/desktop.png)](docs/tulip_desktop.md)

You can [buy a Tulip for $59 US](https://www.makerfabs.com/the-tulip-creative-computer.html) or [build your own Tulip CC](docs/tulip_build.md) for about $25 plus the cost of a display ($50). The hardware for revision 4 of Tulip CC is based on the ESP32-S3 dual core microcontroller running at 240MHz. This single inexpensive chip can support all of Tulip's functionality at low power use. It can last on any USB battery pack or LiPo battery for many hours. 

Tulip CC supports:
- 8.5MB of RAM - 2MB is available to Python, and 1.5MB is available for OS memory. The rest is used for the graphics framebuffers (which you can use as storage) and the firmware cache. 
- 32MB flash storage, as a filesystem accesible in Python (24MB left over after OS in ROM)
- An [AMY](https://github.com/shorepine/amy) stereo 120-voice synthesizer engine running locally, or as a wireless controller for an [Alles](https://github.com/shorepine/alles) mesh. Tulip's synth supports additive and subtractive oscillators, an excellent FM synthesis engine, samplers, karplus-strong, high quality analog style filters, and much more. 
- Text frame buffer layer, 128 x 50, with ANSI support for 256 colors, inverse, bold, underline, background color
- Up to 32 sprites on screen, drawn per scanline, with collision detection, from a total of 32KB of bitmap memory (1 byte per pixel)
- A 1024 (+128 overscan) by 600 (+100 overscan) background frame buffer to draw arbitrary bitmaps to, or use as RAM, and which can scroll horizontally / vertically
- WiFi, access http via Python requests or TCP / UDP sockets 
- Adjustable display clock and resolution, defaults to 30 FPS at 1024x600.
- 256 colors
- Can load PNGs from disk to set sprites or background, or generate bitmap data from code
- Built in code and text editor
- Built in BBS chat room and file transfer area called **TULIP ~ WORLD**
- USB keyboard support
- Capactive multi-touch support (mouse on Tulip Desktop)
- MIDI input and output
- I2C / Grove / Mabee connector, compatible with [many I2C devices like joysticks, keyboard, GPIO, DACs, ADCs, hubs](docs/tulip_api.md#i2c--grove--mabee)
- 575mA power usage @ 5V including display, at medium display brightness, can last for hours on LiPo, 18650s, or USB battery pack 

I've been working on Tulip on and off for years over many hardware iterations and hope that someone out there finds it as fun as I have, either making things with Tulip or working on Tulip itself. I'd love feedback, your own Tulip experiments or pull requests to improve the system.

![Tulip](https://raw.githubusercontent.com/shorepine/tulipcc/main/docs/pics/tulip4.png)


## T-Deck Tulip CC

![T-Deck](docs/pics/tdeck_editor.jpg)

A **new** small option: get yourself a [T-Deck](https://www.aliexpress.us/item/3256805505920840.html?gatewayAdapt=glo2usa4itemAdapt) and install Tulip CC on it directly! [Check out our T-Deck page for more detail.](tulip/tdeck/README.md)


## Getting started

Once you've [bought a Tulip](https://www.makerfabs.com/the-tulip-creative-computer.html), [built a Tulip](docs/tulip_build.md) or [installed Tulip Desktop](docs/tulip_desktop.md), you'll see that Tulip boots right into a Python prompt and all interaction with the system happens there. You can make your own Python programs with Tulip's built in editor and execute them, or just experiment on the Tulip REPL prompt in real time.

[**See the full Tulip API**](docs/tulip_api.md) for more details on all the graphics, sound and input functions.

Below are a few getting started tips and small examples. The [full API](docs/tulip_api.md) page has more detail on everything you can do on a Tulip. [**See a more complete getting started page**](docs/getting_started.md) or [**a music making tutorial**](docs/music.md) as well!



```python
# Run a saved Python file. Control-C stops it
cd('ex') # The ex folder has a few examples and graphics in it
execfile("parallax.py")
# If you want to run a Tulip package (folder with other files in it)
run("game")
```

### The Tulip Editor

Tulip ships with a text editor, based on pico/nano. It supports syntax highlighting, search, save/save-as. 

```python
# Opens the Tulip editor to the given filename. 
edit("game.py")
```

![Editor](https://raw.githubusercontent.com/shorepine/tulipcc/main/docs/pics/editor.png)


### Input and user interface

Tulip supports USB keyboard input and touch input. (On Tulip Desktop, mouse clicks act as touch points.) It also comes with UI elements like buttons and sliders to use in your applications, and a way to run mulitple applications as once using callbacks. More in the [full API](docs/tulip_api.md).

```python
(x0, y0, x1, y1, x2, y2) = tulip.touch()
```

![UI demo](https://raw.githubusercontent.com/shorepine/tulipcc/main/docs/pics/sliders.png)


### Network

Tulip CC has the capability to connect to a Wi-Fi network, and Python's native requests library will work to access TCP and UDP. We ship a few convenience functions to grab data from URLs as well. More in the [full API](docs/tulip_api.md).

```python
# Join a wifi network (not needed on Tulip Desktop)
tulip.wifi("ssid", "password")

# Get IP address or check if connected
ip_address = tulip.ip() # returns None if not connected

# Save the contents of a URL to disk (needs wifi)
bytes_read = tulip.url_save("https://url", "filename.ext")
```


### Music / sound

Tulip comes with the AMY synthesizer, a very full featured 120-oscillator synth that supports FM, PCM, additive synthesis, partial synthesis, filters, and much more. We also provide a useful "music computer" for scales, chords and progressions. More in the [full API](docs/tulip_api.md) and in the [music tutorial.](docs/musid.md) Tulip's version of AMY comes with stereo sound, which you can set per oscillator with the `pan` parameter.

```python
amy.drums() # plays a test song
amy.volume(4) # change volume
amy.reset() # stops all music / sounds playing
```
https://user-images.githubusercontent.com/76612/215893940-658144b7-0c6f-42e2-9836-bd271597aab3.mov


### MIDI

Tulip supports MIDI in and out to connect to external music hardware. You can set up a python callback to respond immediately to any incoming MIDI message. You can also send messages out to MIDI out. More in the [full API](docs/tulip_api.md) and [music tutorial](docs/music.md).

```python
m = tulip.midi_in() # returns bytes of the last MIDI message received
tulip.midi_out((144,60,127)) # sends a note on message
tulip.midi_out(bytes) # Can send bytes or list
```

### Graphics system

The Tulip GPU supports a scrolling background layer, hardware sprites, and a text layer. Much more in the [full API](docs/tulip_api.md).

```python
# Set or get a pixel on the BG
pal_idx = tulip.bg_pixel(x,y)

# Set the contents of a PNG file on the background.
tulip.bg_png(png_filename, x, y)

tulip.bg_scroll(line, x_offset, y_offset, x_speed, y_speed)
```

https://user-images.githubusercontent.com/76612/215895305-7b02ad27-b02a-429a-92ef-f13136e9f9d2.mov


Hardware sprites are supported. They draw over the background and text layer per scanline per frame:

```python
(w, h, bytes) = tulip.sprite_png("filename.png", mem_pos)

...

# Set a sprite x and y position
tulip.sprite_move(12, x, y)
```

https://user-images.githubusercontent.com/76612/215896311-fc0823aa-44bc-4305-85db-a6773db11a98.mov


### Tulip World

Still very much early days, but Tulip supports a native chat and file sharing BBS called **TULIP ~ WORLD** where you can hang out with other Tulip owners. You're able to pull down the latest messages and files and send messages and files yourself. More in the [full API](docs/tulip_api.md).

```python
import world
world.post_message("hello!!") # Sends a message to Tulip World. username required. will prompt if not set
world.upload(filename) # Uploads a file to Tulip World. username required
world.ls() # lists most recent unique filenames/usernames
```

## How to build, compile and help develop Tulip

 * [Get a Tulip!](https://www.makerfabs.com/the-tulip-creative-computer.html)
 * [Build your own Tulip Creative Computer](docs/tulip_build.md) with FOUR different options.
 * [How to compile and flash Tulip hardware](docs/tulip_flashing.md)
 * [How to run or compile Tulip Desktop](docs/tulip_desktop.md)
 * [The full Tulip API](docs/tulip_api.md)
 * [File any code issues or pull requests!](https://github.com/shorepine/tulipcc/issues)

[![shore pine sound systems discord](https://raw.githubusercontent.com/shorepine/tulipcc/main/docs/pics/shorepine100.png) **Chat about Tulip on our Discord!**](https://discord.gg/TzBFkUb8pG)

Two important development guidelines if you'd like to help contribute!

 * Be nice and helpful and don't be afraid to ask questions! We're all doing this for fun and to learn. 
 * Any change or feature must be equivalent across Tulip Desktop and Tulip CC. There are of course limited exceptions to this rule, but please test on hardware before proposing a new feature / change. 

Have fun!

![Tulip](https://raw.githubusercontent.com/shorepine/tulipcc/main/docs/pics/tulip4.png)








