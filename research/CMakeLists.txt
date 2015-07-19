cmake_minimum_required(VERSION 3.2)

project(research)

find_library(AUDIO_UNIT AudioUnit)

set(SOURCES
  main.c
  AudioOut.c
  Synthesizer.c
  SoundFont.c
  Preset.c
  Voice.c
  Filter.c
  Chorus.c
  Reverb.c
  Modulator.c
  Channel.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${AUDIO_UNIT})
