cmake_minimum_required(VERSION 3.2)

project(namidi-synthesizer)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SOURCES
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

if (APPLE)
  set(SOURCES ${SOURCES} osx/AudioOut.c)
endif (APPLE)

add_library(${PROJECT_NAME} ${SOURCES})

if (APPLE)
  find_library(AUDIO_UNIT AudioUnit)
  target_link_libraries(${PROJECT_NAME} ${AUDIO_UNIT})
endif (APPLE)
