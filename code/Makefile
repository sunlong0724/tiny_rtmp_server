
include ../config.MK
OBJS =  amf.o utils.o h264_nalu_queue.o tiny_rtmp_server.o rtmp_server_impl.o h264_2_rtmp.o RingBuffer.o h264_2_rtmp_manager.o
TARGET = libtiny_rtmp_server.so

#libtiny_rtmp_server.so: $(OBJS)
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -shared  -o $@ $^ $(INC_PATH) $(LIBS) $(LIBS_PATH) -lpthread
	mv $@ ../bin

%.o:%.cc
	$(CXX) $(CXXFLAGS) -o $@ -c $<
clean:
	rm -f *.o ../bin/$(TARGET)

.PHONY: $(TARGET) clean
