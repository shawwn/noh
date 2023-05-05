# Newerth of Heroes

## Compilation

### MacOS

```
brew install speex speexdsp giflib openssl fswatch
git clone https://github.com/shawwn/noh --recursive
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
