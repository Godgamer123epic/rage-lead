=====================================================================
  RAGE LEAD  -  melodic rage synth plugin (VST3)   |   START HERE
=====================================================================

WHAT YOU GET
  An 8-voice synth built for rage / melodic rage beats (Ken Carson and
  Playboi Carti style leads): detuned supersaw unison, 8-bit chip waves,
  bell / "alien" tones, and a built-in rage chain: crush, distortion,
  stutter, tape stop, chorus, ping-pong delay, gated reverb and limiter.
  14 factory presets. Works in FL Studio, LMMS and any VST3 host.

YOU DO NOT NEED TO INSTALL VISUAL STUDIO OR WRITE ANY CODE.
GitHub (free) builds the plugin for you in the cloud. About 10-20 minutes.


---------------------------------------------------------------------
 STEP A - Put the project on GitHub (no git commands needed)
---------------------------------------------------------------------
 1. Go to https://github.com and click "Sign up" (free). Confirm your email.
 2. Click the "+" at the top right -> "New repository".
    Name it:  rage-lead     Choose "Public" (public = unlimited free build
    minutes).  Click "Create repository".
 3. Unzip RAGE_LEAD_Project.zip on your computer.
 4. On the new repository page click the link "uploading an existing file".
 5. Open the unzipped folder, press Ctrl+A to select EVERYTHING INSIDE it
    (the folders ".github", "Source", "installer" and all files), and DRAG
    them into the browser window. (Drag the contents, not the outer folder.)
    Wait until every file shows in the list.
 6. Scroll down and click the green "Commit changes" button.

 IMPORTANT - check the hidden ".github" folder uploaded:
   On the repository page you should see a folder named ".github".
   If you DON'T see it (Mac hides folders starting with a dot - press
   Cmd+Shift+. in Finder to show them), do this instead:
     - In the repository click "Add file" -> "Create new file".
     - In the name box type exactly:   .github/workflows/build.yml
       (typing the slashes creates the folders).
     - Open "workflow_backup_build.yml.txt" from the zip with Notepad,
       copy everything, paste it into GitHub, click "Commit changes".


---------------------------------------------------------------------
 STEP B - Let GitHub build it, then download
---------------------------------------------------------------------
 1. Click the "Actions" tab at the top of your repository.
    (If it asks, click "I understand my workflows, go ahead and enable them".)
 2. You will see a run called "Build RAGE LEAD" with a yellow dot (running).
    If there is none: click "Build RAGE LEAD" on the left, then the
    "Run workflow" button -> "Run workflow".
 3. Wait 10-20 minutes until the dot turns into a green check mark.
 4. Click the finished run, scroll to the bottom to "Artifacts" and click
    "RAGE_LEAD_Windows" to download RAGE_LEAD_Windows.zip.
    (A RAGE_LEAD_macOS artifact is there too for Mac users.)


---------------------------------------------------------------------
 STEP C - Unzip
---------------------------------------------------------------------
 Unzip RAGE_LEAD_Windows.zip (right-click -> Extract All). Inside you
 will find:
     RAGE LEAD.vst3        (the plugin - it is a folder, leave it as is)
     INSTALL.bat
     README_START_HERE.txt
     Standalone_Test_App   (optional: runs RAGE LEAD without a DAW)


---------------------------------------------------------------------
 STEP D - Install
---------------------------------------------------------------------
 Double-click INSTALL.bat.
   - If Windows asks "Do you want to allow this app to make changes?" click
     Yes (it needs permission to copy into the VST3 folder).
   - If a blue "Windows protected your PC" box appears: click "More info"
     -> "Run anyway".
 It copies the plugin to:  C:\Program Files\Common Files\VST3
 (the folder FL Studio scans). Wait for "DONE!".


---------------------------------------------------------------------
 STEP E - Find it in FL Studio
---------------------------------------------------------------------
 1. Open FL Studio -> Options -> Manage plugins.
 2. Make sure C:\Program Files\Common Files\VST3 is in the plugin search
    paths (it is by default), then click "Find installed plugins"
    (Start scan).
 3. "RAGE LEAD" appears in the list. Click it and tick the star to
    add it to your favorites.
 4. Add it to your project: in the Channel Rack click the "+" at the
    bottom -> pick "RAGE LEAD" (or Add -> More plugins -> RAGE LEAD).
 5. Play notes with your MIDI keyboard or the Piano Roll. The first preset,
    "Carti Alien Lead", is a ready-to-use rage lead.

 LMMS: copy "RAGE LEAD.vst3" into your VST folder or use
 Tools -> Options -> Directories, then add it from the instrument list.


---------------------------------------------------------------------
 QUICK TOUR
---------------------------------------------------------------------
 - Top bar: preset menu, < > arrows, SAVE, RANDOM (always musical),
   and the BASIC / ADVANCED switch.
 - The 4 big MACRO knobs change the whole vibe:
     RAGE   = distortion + bit-crush + filter drive
     WIDTH  = unison voices + stereo spread + chorus
     SPACE  = delay + reverb
     BRIGHT = filter + distortion tone (middle = neutral)
 - BASIC view: macros + the essential knobs only.
   ADVANCED view: oscillators, filter, envelopes, LFOs, every effect
   and a mod matrix.
 - Double-click any knob to reset it. Mouse wheel works. Hover for help.
 - Mod wheel opens the filter and pushes the distortion.
 - TAPE STOP: hold the TAPE STOP button, or hold MIDI note C0 (note 12).
 - Everything is automatable in FL Studio (right-click a knob ->
   "Create automation clip"). Presets and your project remember all settings.
 - Your saved presets live in:  %APPDATA%\RAGE LEAD\Presets  (XML files)


---------------------------------------------------------------------
 TROUBLESHOOTING
---------------------------------------------------------------------
 "RAGE LEAD doesn't show up in FL Studio"
   - Check that the folder  C:\Program Files\Common Files\VST3\RAGE LEAD.vst3
     exists (it should contain a "Contents" folder).
   - In Manage plugins click "Find installed plugins" again. Tick
     "Verify plugins" off/on and rescan if needed.
   - Close and restart FL Studio, then scan again.
   - Look in the plugin list under "Installed" > "Generators" (it is a synth).

 "Windows blocked the file / plugin won't load"
   - Right-click the downloaded RAGE_LEAD_Windows.zip -> Properties ->
     tick "Unblock" -> OK, then unzip it again and re-run INSTALL.bat.
     (INSTALL.bat also tries to unblock everything automatically.)

 "No sound"
   - Make sure the channel is routed to a mixer track, the MASTER knob
     is up, and you are sending MIDI notes. Try the preset "Plain Init Saw".

 "The GitHub build failed (red X)"
   - Click the failed run -> click the red step -> copy the red error
     lines (or use "..." -> "View raw logs"), paste them to Claude and
     ask for a fix. Then upload the fixed files the same way as Step A
     (choose "Add file -> Upload files", it replaces old versions) and the
     build runs again by itself.

 "I don't see the Actions tab / nothing started"
   - Make sure the ".github" folder exists in the repository (see the note
     at the end of Step A), then use "Run workflow" manually.

 Note on JUCE: this project uses the JUCE framework (downloaded
 automatically at build time) under its free personal / AGPL terms.
 Fine for personal use; read juce.com/legal before selling anything.


---------------------------------------------------------------------
 OPTIONAL - BUILD ON YOUR OWN PC (for people who prefer it)
---------------------------------------------------------------------
 1. Install Visual Studio Community (free) with the "Desktop development
    with C++" workload, plus CMake (included in that workload) and Git.
 2. Open "x64 Native Tools Command Prompt for VS" and run:
        cd path\to\RAGE_LEAD
        cmake -S . -B build -A x64
        cmake --build build --config Release --target RageLead_VST3
 3. The plugin appears in:
        build\RageLead_artefacts\Release\VST3\RAGE LEAD.vst3
    Copy that folder into C:\Program Files\Common Files\VST3 (as admin).
 macOS / Linux:
        cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
        cmake --build build --target RageLead_VST3 --parallel
    On Linux you first need: libasound2-dev libfreetype-dev libx11-dev
    libxrandr-dev libxinerama-dev libxcursor-dev libxext-dev libgl1-mesa-dev
 macOS: after building double-click installer/INSTALL_MAC.command
 (or use the RAGE_LEAD_macOS artifact from GitHub).
