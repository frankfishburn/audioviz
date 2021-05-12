add_executable(
  audioviz 
  main.cpp 
  algorithm/resampler.cpp 
  algorithm/stft.cpp
  audio/file_source.cpp  
  audio/player.cpp 
  video/framebuffer.cpp 
  video/shader_program.cpp 
  video/window.cpp 
  vfx/liquid/effect.cpp
)
target_include_directories(audioviz PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(audioviz PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(audioviz PUBLIC ${FFTW_INCLUDE_DIR})
target_link_libraries(audioviz SDL2 SDL2_image)
target_link_libraries(audioviz ${OPENGL_LIBRARIES} GLEW)
target_link_libraries(audioviz avcodec avformat avutil avdevice bz2 swresample)
target_link_libraries(audioviz ${FFTW_LIBS} ${FFTWF_LIBS} ${SPECTROGRAM_LIB} )
install(TARGETS audioviz DESTINATION bin)
