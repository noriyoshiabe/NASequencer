cmake_minimum_required(VERSION 3.2)

project(namidi-synthesizer)

find_library(AUDIO_UNIT AudioUnit)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCES
  AudioOut.c
  Synthesizer.c
  SoundFont.c
  Preset.c
  Voice.c
  Envelope.c
  LFO.c
  Filter.c
  Chorus.c
  Reverb.c
  Modulator.c
  Channel.c
)

add_library(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${AUDIO_UNIT})
