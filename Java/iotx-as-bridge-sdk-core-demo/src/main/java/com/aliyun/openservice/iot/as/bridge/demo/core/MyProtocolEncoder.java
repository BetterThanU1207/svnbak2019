package com.aliyun.openservice.iot.as.bridge.demo.core;

import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.MessageToByteEncoder;

public class MyProtocolEncoder extends MessageToByteEncoder<DdpProtocolDatapkg> {
    @Override
    protected void encode(ChannelHandlerContext ctx, DdpProtocolDatapkg msg, ByteBuf out) {
        //out.writeInt((int)msg.value());
    }
}
