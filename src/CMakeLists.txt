add_executable(
  audioviz
  main.cpp
  algorithm/resampler.cpp
  algorithm/stft.cpp
  audio/file_source.cpp
  audio/player.cpp
  video/framebuffer.cpp
  video/shader.cpp
  video/shader_program.cpp
  video/vertex_array.cpp
  video/vertex_buffer.cpp
  video/window.cpp
  visuals/eclipse/eclipse.cpp
  visuals/liquid/liquid.cpp
)
target_include_directories(audioviz PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(audioviz PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(audioviz PUBLIC ${FFTW_INCLUDE_DIR})
target_link_libraries(audioviz SDL2 SDL2_image)
target_link_libraries(audioviz ${OPENGL_LIBRARIES} GLEW)
target_link_libraries(audioviz avcodec avformat avutil avdevice bz2 swresample)
target_link_libraries(audioviz ${FFTW_LIBS} ${FFTWF_LIBS} ${SPECTROGRAM_LIB} )
install(TARGETS audioviz DESTINATION bin)
