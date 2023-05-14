file (GLOB src *.cxx)

add_library(NeBullet SHARED ${src})

target_link_libraries(NeBullet NekoEngine Bullet3Common Bullet3Collision LinearMath)
if(APPLE)
	target_link_libraries(NeBullet "-Wl,-undefined,dynamic_lookup")
endif()
