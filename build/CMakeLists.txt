PROJECT(main)


SET(SRC_LIST ../src/main.cpp
             # 3rd ----
             ../src/lib/3rd/json11/json11.cpp
             # helper ----
             ../src/lib/helper/Webpage.cpp
             ../src/lib/helper/Time.cpp
             ../src/lib/helper/CmdlineOption.cpp
             ../src/lib/helper/Misc.cpp
             # self ----
             ../src/lib/self/AllArtists.cpp
             ../src/lib/self/Artist.cpp
             ../src/lib/self/Album.cpp
             ../src/lib/self/Song.cpp
             ../src/lib/self/Pass.cpp )

# linux
#>>>>>>>>>>>>>>>>>>>>>>

## debug
#SET(CMAKE_CXX_COMPILER "clang++")
#SET(CMAKE_CXX_FLAGS "-std=c++11 -Werror -Weverything -Wno-documentation -Wno-disabled-macro-expansion -Wno-float-equal -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-global-constructors -Wno-exit-time-destructors -Wno-missing-prototypes -Wno-padded -Wno-old-style-cast -Wno-weak-vtables")
#SET(CMAKE_BUILD_TYPE debug)
#ADD_EXECUTABLE(main ${SRC_LIST})
#TARGET_LINK_LIBRARIES(main curl pthread)

# release
SET(CMAKE_CXX_COMPILER "g++")
SET(CMAKE_CXX_FLAGS "-std=c++11 -O3")
SET(CMAKE_BUILD_TYPE release)
ADD_EXECUTABLE(yosong ${SRC_LIST})
TARGET_LINK_LIBRARIES(yosong curl pthread)
INSTALL(PROGRAMS yosong DESTINATION /usr/bin/)

#<<<<<<<<<<<<<<<<<<<<<<


## cygwin
##>>>>>>>>>>>>>>>>>>>>>>

#SET(CMAKE_CXX_COMPILER "g++")
#SET(CMAKE_CXX_FLAGS "-std=c++11 -O3 -s -DCYGWIN")
#SET(CMAKE_BUILD_TYPE release)
#ADD_EXECUTABLE(yosong ${SRC_LIST})
#target_link_libraries(yosong /bin/cygcurl-4.dll)
#target_link_libraries(yosong /lib/libiconv.a)

##<<<<<<<<<<<<<<<<<<<<<<


## osX
##>>>>>>>>>>>>>>>>>>>>>>

#SET(CMAKE_CXX_COMPILER "g++")
#SET(CMAKE_CXX_FLAGS "-std=c++11 -O3")
#SET(CMAKE_BUILD_TYPE release)
#ADD_EXECUTABLE(yosong ${SRC_LIST})
#TARGET_LINK_LIBRARIES(yosong curl pthread iconv)
#INSTALL(PROGRAMS yosong DESTINATION /usr/bin/)

##<<<<<<<<<<<<<<<<<<<<<<
