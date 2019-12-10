package com.aliyun.openservice.iot.as.bridge.demo.core;

import com.google.common.util.concurrent.ThreadFactoryBuilder;
import io.netty.bootstrap.ServerBootstrap;
import io.netty.channel.*;
import io.netty.channel.nio.NioEventLoopGroup;
import io.netty.channel.socket.SocketChannel;
import io.netty.channel.socket.nio.NioServerSocketChannel;
import io.netty.handler.codec.LengthFieldBasedFrameDecoder;
import io.netty.handler.logging.LogLevel;
import io.netty.handler.logging.LoggingHandler;

import java.util.concurrent.ThreadFactory;

/**
 * Created by Administrator on 2019/11/5.
 */
public class NettyServer {
    private int port; //服务器端端口号

    public NettyServer(int port) {
        this.port = port;
    }
    //耗时业务处理线程池工厂
    final ThreadFactory threadFactory = new ThreadFactoryBuilder()
            .setNameFormat("netty-context-business-%d")
            .setDaemon(false)
            .build();

    public void run() throws Exception {
        EventLoopGroup bossGroup = new NioEventLoopGroup();
        EventLoopGroup workerGroup = new NioEventLoopGroup();

        NioEventLoopGroup business = new NioEventLoopGroup(50,threadFactory);

        try {
            ServerBootstrap b = new ServerBootstrap();
            b.group(bossGroup, workerGroup)
                    .channel(NioServerSocketChannel.class)
                    .option(ChannelOption.SO_BACKLOG, 128)
                    .childOption(ChannelOption.SO_KEEPALIVE, true)
                    .childHandler(new ChannelInitializer<SocketChannel>() {
                        @Override
                        public void initChannel(SocketChannel ch) {
                            ChannelPipeline pipeline = ch.pipeline();
                            //往pipeline链中添加一个日志记录器
                            //pipeline.addLast(new LoggingHandler(LogLevel.INFO));

                            //往pipeline链中添加一个解码器：数据包最长不超过1040Byte，包长度由2byte标识，包长度偏移2Bytes
                            LengthFieldBasedFrameDecoder lbDecoder = new LengthFieldBasedFrameDecoder(1040,2,2,-4,0);
                            pipeline.addLast("decoder", lbDecoder);//new MyProtocolDecoder()
                            //往pipeline链中添加自定义的handler(业务处理类)，并关联到业务线程池，用于处理耗时任务
                            pipeline.addLast(business,"handler",new AdapterServerHandler());//
                        }
                    });
            BridgeBasicV1.getLogger().warn("======== Netty Server start success =========");
            ChannelFuture f = b.bind(port).sync();
            f.channel().closeFuture().sync();
        } finally {
            workerGroup.shutdownGracefully();
            bossGroup.shutdownGracefully();
            BridgeBasicV1.getLogger().warn("======== Netty Server close =========");
        }
    }
}