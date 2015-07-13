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
  IIRFilter.c
  Chorus.c
  Reverb.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${AUDIO_UNIT})
