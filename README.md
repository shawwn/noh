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

### Building NoH on Windows

Coming soon. Building on Windows should be roughly similar to the macOS steps below, but you'll need to install Visual Studio and a few other things that I'll need to document.

### Building NoH on macOS

##### Install dependencies
```
brew install llvm speex speexdsp giflib openssl fswatch fileicon
```
##### Clone the repository (`--recursive` is important!)
```
git clone --recursive https://github.com/shawwn/noh
```
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
