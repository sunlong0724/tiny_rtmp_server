
include ../config.MK
OBJS =  main.o hikConnect.o

TARGET = test_h264_2_rtmp

INC_PATH = -I . -I ../code -I ./hikvision 

LIBS_PATH = -L ../bin
LIBS = -Wl,-rpath=./hikvision/lib/HCNetSDKCom -lhcnetsdk -lhpr -lHCCore

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(INC_PATH) $(LIBS) $(LIBS_PATH) -ltiny_rtmp_server -lpthread
	mv $@ ../bin

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $< $(INC_PATH) 
%.o:%.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $< $(INC_PATH) 


clean:
	rm -f *.o ../bin/$(TARGET)
	rm -f *.o ../bin/$(TARGET)_hik
.PHONY: $(TARGET) clean
