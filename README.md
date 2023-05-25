# Newerth of Heroes

<img width="320" alt="image" src="https://user-images.githubusercontent.com/59632/236418122-c052ff68-467a-470f-9f90-fc53d51de862.png">

An open source implementation of [dota](https://www.youtube.com/watch?v=qTsaS1Tm-Ic&ab_channel=BASSHUNTER). 

Cross platform; multiplayer; lobby system; map editor; replay system; semicolon. Builds in 3 minutes flat on an M1 Air.

## About

Intended to be the game engine that my 13yo self wished he'd had. Think of it as a baseline "here's a fully working game engine, along with an actual game" reference that you can use for whatever you want. It's small enough that you can understand it, build it yourself, and customize.

My long term goal is to implement a custom game system reminiscent of the StarCraft 1 "Use Map Settings" era. There was [something magical about it](https://www.youtube.com/watch?v=hu_ekZfW6wE&t=76s&ab_channel=RedDevouringOne) that modern "custom game" attempts don't really capture. If you remember it, you know what I mean -- point to any modern equivalent that makes you feel the same spark of joy of joining a random lobby and discovering that it's actually a weirdly-detailed esoteric RPG that some 14yo crafted in StarEdit, or an intense tower defense experience that you didn't expect.

<img width="595" alt="image" src="https://user-images.githubusercontent.com/59632/236426949-89ac5d63-f391-4701-978c-63f93e31cb5d.png">

It also makes my heart ache that your only realistic choices for becoming a professional gamedev circa 2023 is to build everything from scratch yourself (Celeste) or to spend years learning Unreal Engine or Unity. Good luck understanding the inner workings of those behemoths, much less getting UE to build.

So hopefully this will give you a leg up as a lone wolf gamedev.

<img width="595" alt="image" src="https://user-images.githubusercontent.com/59632/236428981-60051d51-eeda-4e99-9846-e5024ae8f37f.png">

Ultimately, I expect this to take somewhere between three months and three years, and for roughly seven people to care about it. But if you're one of those seven, know that you'll have all my heart and soul pushing you forward, for whatever it's worth. I fell in love with the K2 engine when I worked at S2 in 2010. Perhaps one or two others might too.

## Media

### Newerth dev stream #1

https://youtu.be/VBj0RcpxCIc?t=132

I wanted to give hackers a sense of how it feels to work with the engine, so I recorded about an hour of random work.

Be sure to read the chapter titles as you watch; it's a detailed blow-by-blow of my thought process as I went.

### "I was a former HoN dev. AMA"

https://www.reddit.com/r/DotA2/comments/asc14j/i_was_a_former_hon_dev_ama/

This was an AMA I did when S2 officially shuttered Hon after Valve steamrolled them. The timestamp says four years ago, but it feels like a decade.

I originally joined S2 because I loved the game, and a certain bug was so frustrating that I simply had to annihilate it. Joining the company was the only way I could, so I did.

## License

All code and assets are MIT licensed, to the extent that I'm authorized to do so. Which is to say, not at all. But nobody cares at this point.

## Compiling NoH

### Building NoH on macOS

##### Install macOS dependencies

Install [Homebrew](https://brew.sh/):
```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Install the dependencies:

```
brew install cmake llvm ninja speex speexdsp giflib libpng libjpeg openssl@1.1 freetype fswatch fileicon
```

##### Clone the repository (`--recursive` is important!)

```
git clone --recursive https://github.com/shawwn/noh
```

##### Build on macOS with CMake

```
cd noh
mkdir build
cd build
cmake ..
cmake --build .
```

Things you can do:

- Run `./K2\ Model\ Viewer` and play with some effects
- Run `./NoH` and click "Local Game" to start a game
- Run `./NoH\ Editor`


### Building NoH on macOS with [CLion](https://www.jetbrains.com/clion/download/#section=mac)

##### Open the repo in [CLion](https://www.jetbrains.com/clion/download/#section=mac):
```
cd noh
clion .
```
 ##### Once CLion is open

- Open Project view (press `Cmd-1`)

- Open `CMakeLists.txt`

- Click "Load CMake project" in the upper right

<img width="1512" alt="image" src="https://user-images.githubusercontent.com/59632/236375944-0f028a9f-79b9-4939-b063-a9657fafe59b.png">

Ignore the scary-looking errors and just click the play button (`Run -> Run 'NoH'`)

<img width="1398" alt="image" src="https://user-images.githubusercontent.com/59632/236376120-22f8d300-5b8c-4615-bf48-f3c2b03256cb.png">

If things go well, NoH will launch. Congratulations!

<img width="1440" alt="image" src="https://user-images.githubusercontent.com/59632/236376813-77395c90-89e6-4713-9a6b-09859caf33f9.png">

#### If things go wrong

Open CMake tab, click "Reset Cache and Reload Project" (the icon in the upper left of the tab with two arrows)

<img width="889" alt="image" src="https://user-images.githubusercontent.com/59632/236376625-105f0786-06b2-4032-bdaf-7d7f97144d17.png">

Build -> Rebuild all in Release

<img width="297" alt="image" src="https://user-images.githubusercontent.com/59632/236376681-1eff6c59-43d8-462f-8a84-20ebe6f341b1.png">

Run -> Run NoH, or just click the play button

If things still aren't working, [post an issue](https://github.com/shawwn/noh/issues) or DM me on Twitter: [@theshawwn](https://twitter.com/theshawwn)

### Building NoH on Windows

##### Install Git for Windows

Install [Git for Windows](https://gitforwindows.org/)

During setup, you can leave everything as default, **except make sure you check "Enable symbolic links"**:

<img width="514" alt="image" src="https://github.com/shawwn/noh/assets/59632/0d03e2a0-dac6-4176-b427-6c30c3dcec5e">

##### Install CMake for Windows

[Download CMake](https://cmake.org/download/) (probably choose "Windows x64 Installer")


<img width="498" alt="image" src="https://github.com/shawwn/noh/assets/59632/7735c747-a017-4aae-b775-042fe48c7bfc">

During setup, click "Add CMake to the system PATH for the current user":

<img width="497" alt="image" src="https://github.com/shawwn/noh/assets/59632/a7e6fde2-ef1e-4e86-b753-416ee6390b4d">

##### Install Visual Studio

If you have Visual Studio 2019 or later instsalled, you can skip this step.

Install [Visual Studio 2022 Community](https://visualstudio.microsoft.com/downloads/):
<img width="478" alt="image" src="https://github.com/shawwn/noh/assets/59632/4fde72d4-204b-4a16-a7d6-40e3b2640de2">

During setup, choose "Desktop development with C++":
<img width="1238" alt="image" src="https://github.com/shawwn/noh/assets/59632/ebbeaefd-9d92-4bff-a3e3-f5dd6116e0d3">

(If you want to save around 6GB, you can you can uncheck ".NET desktop development")

Check "Install", then wait awhile.

Eventually you'll see this:
<img width="985" alt="image" src="https://github.com/shawwn/noh/assets/59632/ad9953d8-685d-4489-b251-a8d7b0daf1ba">

##### Install CLion

Install CLion: [https://www.jetbrains.com/clion/download/](https://www.jetbrains.com/clion/download/)

##### Build with CLion

Open CLion and clone the repo:
<img width="804" alt="image" src="https://github.com/shawwn/noh/assets/59632/67ec32d6-b4b3-4287-9281-6cb7bc8fc009">

Click "Trust Project"
<img width="505" alt="image" src="https://github.com/shawwn/noh/assets/59632/49fdfb05-fb27-46d3-ae26-d4539ef233ad">

Configure Microsoft Defender:
<img width="310" alt="image" src="https://github.com/shawwn/noh/assets/59632/6826b757-5465-4a0e-ab07-b114c310c118">

Go to File -> Settings, then "Build, Execution, Deployment" and click "Toolchains". Verify that your settings look similar to this:
<img width="1247" alt="image" src="https://github.com/shawwn/noh/assets/59632/cf06074e-1935-4e92-a8b2-5cc232c72688">

Open the Project tool window (Press `Alt-1`):
<img width="501" alt="image" src="https://github.com/shawwn/noh/assets/59632/d7760437-8338-4ca0-8b3b-10345c50619d">

Open CMakeLists.txt, then click "Load CMake project" in the upper right:
![image](https://github.com/shawwn/noh/assets/59632/eb0f2eda-3d61-4b32-b529-b1a9ea073d52)

You'll see some errors:
<img width="1247" alt="image" src="https://github.com/shawwn/noh/assets/59632/467b950c-6f3c-48fb-a9c7-b39c704fc769">

Click the Vcpkg tab:
<img width="553" alt="image" src="https://github.com/shawwn/noh/assets/59632/0d2b9025-8096-4f36-8985-bd445142510c">

Click the plus sign ("Add vcpkg")
<img width="206" alt="image" src="https://github.com/shawwn/noh/assets/59632/e9cc456a-c127-457b-8082-949e65dc9f66">

Verify that "Add vcpkg integration to existing CMake profiles" is selected, then click ok:
<img width="335" alt="image" src="https://github.com/shawwn/noh/assets/59632/18ab935b-bf8d-405d-8560-8130ce486014">

Vcpkg will now start building the required dependencies:
<img width="855" alt="image" src="https://github.com/shawwn/noh/assets/59632/db1b4bf2-f744-48df-bb21-1e1a6f5ad295">

You can click the "CMake" tab to watch its progress:
<img width="1052" alt="image" src="https://github.com/shawwn/noh/assets/59632/ae0bdfbb-8d0f-4ce3-883e-b042e12f2c4b">


<img width="1240" alt="image" src="https://github.com/shawwn/noh/assets/59632/72e1e901-abf8-4c1a-8ff4-43d598f01c02">

Wait awhile (~40 minutes, sorry; luckily you only need to do this once, ever).

Eventually "Loading CMake project..." in the status bar will disappear.

<img width="1260" alt="image" src="https://github.com/shawwn/noh/assets/59632/29028649-0feb-4a7c-abaa-32daab3c98b4">

Click the run button.
<img width="683" alt="image" src="https://github.com/shawwn/noh/assets/59632/b43a48cf-9432-46c9-b9c3-ec9ddbfda110">

The engine will start building:
<img width="1264" alt="image" src="https://github.com/shawwn/noh/assets/59632/53d043bf-fdc9-4bab-91a7-41a0f8c90f02">

With any luck, it'll launch. You're done!

(To get into a game, change the configuration to NoH and run it. Then click "Local Game" -> Create Game, join a lobby
slot, start game, and choose a hero.)
