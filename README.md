# Newerth of Heroes

<img width="320" alt="image" src="https://user-images.githubusercontent.com/59632/236402360-549155ac-e95f-4676-af08-c9d0689c8311.png">

An open source implementation of [dota](https://www.youtube.com/watch?v=qTsaS1Tm-Ic&ab_channel=BASSHUNTER). 

Cross platform; multiplayer; lobby system; map editor; replay system; semicolon.

## About

Intended to be the game engine that my 13yo self wished he'd had. Think of it as a baseline "here's a fully working game engine, along with an actual game" reference that you can use for whatever you want. It's small enough that you can understand it, build it yourself, and customize whatever you want.

It also makes my heart ache that your only real choice nowadays for becoming a gamedev is to build everything from scratch yourself (Celeste) or to spend years learning Unreal Engine or Unity. Good luck understanding the inner workings of those.

## Media

Newerth dev stream #1: https://youtu.be/VBj0RcpxCIc?t=132

(Presented without explanation, and filmed at a dog park. Be sure to read the chapter titles as you watch; it's a detailed blow-by-blow of my thought process as I was hacking on the engine.)

See also: ["I was a former HoN dev. AMA"](https://www.reddit.com/r/DotA2/comments/asc14j/i_was_a_former_hon_dev_ama/)

## License

All code and assets are MIT licensed, to the extent that I'm authorized to do so. Which is to say, not at all. But nobody cares at this point.

## Compiling NoH

Havent tested Windows in awhile, so it might be broken. But I wasn't intending to announce this tonight, let alone at all, so maybe check back in a week or a few years and see if it's polished yet.

### MacOS

Install dependencies:
```
brew install speex speexdsp giflib openssl fswatch
```
Clone the repository (`--recursive` is important!)
```
git clone --recursive https://github.com/shawwn/noh
```
Open the repo in [CLion](https://www.jetbrains.com/clion/download/#section=mac):
```
cd noh
clion .
```
Once CLion is open:
- Open Project view (press `Cmd-1`)
- Open `CMakeLists.txt`
- Click "Load CMake project" in the upper right
  <img width="1512" alt="image" src="https://user-images.githubusercontent.com/59632/236375944-0f028a9f-79b9-4939-b063-a9657fafe59b.png">
- Ignore the scary-looking errors and just click the play button (`Run -> Run 'NoH'`):
  <img width="1398" alt="image" src="https://user-images.githubusercontent.com/59632/236376120-22f8d300-5b8c-4615-bf48-f3c2b03256cb.png">

If things go well, NoH will launch. Congratulations!
<img width="1440" alt="image" src="https://user-images.githubusercontent.com/59632/236376813-77395c90-89e6-4713-9a6b-09859caf33f9.png">

#### If things go wrong:
- Open CMake tab, click "Reset Cache and Reload Project" (the icon in the upper left of the tab with two arrows)
  <img width="889" alt="image" src="https://user-images.githubusercontent.com/59632/236376625-105f0786-06b2-4032-bdaf-7d7f97144d17.png">
- Build -> Rebuild all in Release
  <img width="297" alt="image" src="https://user-images.githubusercontent.com/59632/236376681-1eff6c59-43d8-462f-8a84-20ebe6f341b1.png">
- Run -> Run NoH, or just click the play button

If things still aren't working, [post an issue](https://github.com/shawwn/noh/issues) or DM me on Twitter: [@theshawwn](https://twitter.com/theshawwn)
