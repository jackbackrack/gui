CC = gcc
# CXX = g++-4.8

OS2 := $(strip $(shell uname))
ifeq ($(OS2), Darwin)
  # Mac OSX
  $(info Compiling for Darwin)
  LIBS = -lobjc -lopencv_core -lopencv_highgui -lopencv_video -lopencv_imgproc -lopencv_objdetect -lopencv_calib3d -lode -framework GLUT -framework OpenGL -L/opt/local/lib -L/usr/local/lib -lportmidi -lportaudio #-lglaux # -pg # -framework Foundation
  #HLLIBS = 
  AUDIO_OBJS = port_midi.o port_audio.o
  INCS = -I/usr/local/include -I/opt/local/include/OpenEXR -I/opt/local/include/ -I/usr/local/include/OpenEXR
  FLAGS = -std=c++0x -Wno-write-strings -g -O3 -D MACOSX=1 -DIS_CV -D IS_AUDIO=1 $(INCS) # -pg  
else
  OS := $(strip $(shell uname -o))
  ifeq ($(OS), GNU/Linux)
    $(info Compiling for GNU/Linux)
    INCS = -I/usr/include -I/usr/include/OpenEXR  -DIS_CV
    LIBS = -lglut -lGL -lGLU -lm -lopencv_imgproc -lopencv_core -lopencv_highgui # -lcv -lcvaux -lhighgui # -L"/usr/local/share/ipp/sharedlib"  -lippi -lippcore -lippcv -lippcc -lguide -lippvm # -lglaux # -pg
    HLLIBS = -lm
    FLAGS = -std=c++0x -g -Wno-write-strings $(INCS)# -pg  -O3
  else
    ifeq ($(OS), Cygwin)
      # Cygwin under Windows
      $(info Compiling for Cygwin)
      INCS =  $(DEFS) -Ic:/jrb/ode-0.8/include -I c:/progra~1/opencv/cxcore/include  -Ic:/progra~1/opencv/cv/include -Ic:/progra~1/opencv/cvaux/include -Ic:/progra~1/opencv/otherlibs/highgui 
      SLIBS = c:/jrb/ode-0.8/ode/src/libode.a  c:/progra~1/opencv/lib/cv.lib c:/progra~1/opencv/lib/cvaux.lib c:/progra~1/opencv/lib/highgui.lib c:/progra~1/opencv/lib/cxcore.lib
      LIBS =  $(SLIBS) libportaudio.win32.a libportmidi.win32.a -lglut32 -lopengl32 -lglu32 -lglaux -lwinmm -lgdi32 -lstdc++ # -pg
      #HLLIBS = 
      FLAGS = $(INCS) -std=c++0x -O3 -g -DIS_CV# -pg  
    else
      # unknown; fail noisily
      $(error Unknown operating system '$(OS)'; please rerun make with OS set to 'GNU/Linux', 'Darwin', or 'Cygwin')
    endif
  endif
endif

all: gui lisp lay app.a console.a gui.a

install: all
	cp -p *.a ../lib; cp gui lisp lay ../bin

clean:
	rm *.o *.a

props.o: props.cpp props.h

cmds.o: cmds.cpp

utils.o: utils.cpp

keys.o: keys.cpp keys.h

lisp.o: lisp.cpp lisp.h

draw.o: draw.cpp draw.h

reader.o: reader.cpp lisp.h reader.h

port_midi.o: port_midi.h

port_audio.o: port_audio.h

gfx.o: gfx.cpp gfx.h

viz.o: viz.cpp viz.h

pink.o: 

pink: pink.o
	$(CC) -o pink pink.o -lportaudio 

sine.o: 

sine: sine.o
	$(CC) -o sine sine.o -lportaudio 

timer.o: timer.cpp timer.h

gui.o: gui.cpp props.h cmds.h keys.h

console.a: props.o timer.o utils.o lisp.o
	ar -v -r -u console.a $^

gui.a: props.o timer.o utils.o lisp.o cmds.o keys.o port_midi.o vec_gfx.o
	ar -v -r -u gui.a $^

app.a: props.o timer.o utils.o lisp.o cmds.o keys.o draw.o reader.o trackball.o viz.o gfx.o vec_gfx.o lay.o $(AUDIO_OBJS) 
	ar -v -r -u app.a $^ 

gui: gui.o console.a gui.a
	$(CXX) $(LDFLAGS) -o gui gui.o gui.a  $(LIBS) 

lisp: lisp.o utils.o reader.o lisp-test.o
	$(CXX) $(LDFLAGS) -o lisp lisp.o utils.o reader.o lisp-test.o $(LIBS) 

trackball.o: trackball.cpp trackball.h

lay.o: lay.cpp lay.h lisp.h viz.h

lay-test.o: lay-test.cpp lay.h lisp.h

lay: lay-test.o app.a 
	$(CXX) $(LDFLAGS) -o lay lay-test.o app.a  $(LIBS)

.c.o:
	$(CC) $(FLAGS) -c $<

.cpp.o:
	$(CXX) $(FLAGS) -c $<

