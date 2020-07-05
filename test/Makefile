
include ../config.MK

OBJS = main.o
INC_PATH = -I . -I ../code  

ifdef HIK
OBJS =  main.o hikConnect.o
INC_PATH = -I . -I ../code -I ./hikvision 
LIBS = -Wl,-rpath=./hikvision/lib/HCNetSDKCom -lhcnetsdk -lhpr -lHCCore
endif

TARGET = test_h264_2_rtmp

LIBS_PATH = -L ../bin

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INC_PATH) $(LIBS) $(LIBS_PATH) -ltiny_rtmp_server -lpthread
	mv $@ ../bin

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $< $(INC_PATH) 
%.o:%.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $< $(INC_PATH) 


clean:
	rm -f *.o ../bin/$(TARGET)
.PHONY: $(TARGET) clean
