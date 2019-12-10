package com.aliyun.openservice.iot.as.bridge.demo.core;

import java.io.*;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

import com.aliyun.iot.as.bridge.core.BridgeBootstrap;
import com.aliyun.iot.as.bridge.core.config.ConfigFactory;
import com.aliyun.iot.as.bridge.core.handler.DownlinkChannelHandler;
import com.aliyun.iot.as.bridge.core.model.DeviceIdentity;
import com.aliyun.iot.as.bridge.core.model.Session;
import com.google.common.util.concurrent.ThreadFactoryBuilder;
import io.netty.buffer.ByteBuf;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import com.google.gson.Gson;
import org.springframework.stereotype.Component;

/**
 * Bridge Gateway Demo for IoT Link Platform Senior Version
 */
@Component
public class BridgeBasicV1 {
    /**
     * bridge server initializer
     */
	private static Logger log = LoggerFactory.getLogger(BridgeBasicV1.class);

	public static Logger getLogger(){return log;}

    private static BridgeBootstrap bridgeBootstrap = new BridgeBootstrap();

    public static TerminalsConfMgr terminalsConfMgr=null;

    /**
     * self-define topic template created in IoT Platform Web Console
     */
    //private final static String TOPIC_TEMPLATE_USER_DEFINE = "/%s/%s/user/update";
    
    //private final static String PROP_POST_PAYLOAD_TEMPLATE = "Hello IoT Bridge";
    
    private static ExecutorService executorService  = new ThreadPoolExecutor(
        Runtime.getRuntime().availableProcessors(),
        Runtime.getRuntime().availableProcessors() * 2,
        60, TimeUnit.SECONDS,
        new LinkedBlockingQueue<>(1000),
        new ThreadFactoryBuilder().setDaemon(true).setNameFormat("bridge-downlink-handle-%d").build(),
        new ThreadPoolExecutor.AbortPolicy());

    //public static ExecutorService getExecutorService(){return executorService;}
    /**
     * netty server initializer
     */

    /**
     * Main
     */
	public static void main(String[] args) throws Exception {
	    //start bridge server,Use application.conf & devices.conf by default

        bridgeBootstrap.bootstrap(new DownlinkChannelHandler() {
            @Override
            public boolean pushToDevice(Session session, String topic, byte[] payload) {
            	//get downlink message from cloud
                executorService.submit(() -> handleDownLinkMessage(session, topic, payload));
                return true;
            }

            @Override
            public boolean broadcast(String s, byte[] bytes) {
                return false;
            }
        });

        //初始化终端设备信息
//        String filepath = ResourceUtils.getFile("terminals.json") ;

//        File jsonFile = ResourceUtils.getFile("terminals.json");//new File(filepath);
        InputStream readIs = ResourceUtils.getInputStream("terminals.json");
        InputStreamReader read = new InputStreamReader(readIs, "utf-8");
        BufferedReader bufferedReader = new BufferedReader(read);
        Gson gson = new Gson();
//        Reader reader = new InputStreamReader(new FileInputStream(jsonFile),"utf-8");
        terminalsConfMgr = gson.fromJson(bufferedReader,TerminalsConfMgr.class);

        //String s="7b0100163133393038333230303533de846186138b7b";
        //byte[] bs = hexToByteArray(s);

        //log.info("bs={}",bs.toString());//Integer.toHexString(ret[0]),Integer.toHexString(ret[1]));

        log.warn("======== Bridge bootstrap success =========");
        new NettyServer(getNettyServerPort()).run();
	}


	private static void handleDownLinkMessage(Session session, String topic, byte[] payload) {
        String content = new String(payload);
        log.info("Get DownLink message, session:{}, topic:{}, content:{}", session, topic, content);
        Object channel = session.getChannel();
        String originalIdentity = session.getOriginalIdentity();
        //for example, you can send the message to device via channel, it depends on you specific server implementation
    }

    //配置端口的设备信息中PK、DN不得为空
    private static int getNettyServerPort(){
        DeviceIdentity deviceIdentity = ConfigFactory.getDeviceConfigManager().getDeviceIdentity("jswaterNettyPort");
        if(deviceIdentity!=null) {
            String port = deviceIdentity.getDeviceSecret();
            return Integer.parseInt(port);
        }else{
            return 9999;
        }
    }

    /**
     * hex字符串转byte数组
     * @param inHex 待转换的Hex字符串
     * @return  转换后的byte数组结果
     */
    public static byte[] hexToByteArray(String inHex){
        int hexlen = inHex.length();
        byte[] result;
        if (hexlen % 2 == 1){
            //奇数
            hexlen++;
            result = new byte[(hexlen/2)];
            inHex="0"+inHex;
        }else {
            //偶数
            result = new byte[(hexlen/2)];
        }
        int j=0;
        for (int i = 0; i < hexlen; i+=2){
            result[j]=hexToByte(inHex.substring(i,i+2));
            j++;
        }
        return result;
    }

    /**
     * Hex字符串转byte
     * @param inHex 待转换的Hex字符串
     * @return  转换后的byte
     */
    public static byte hexToByte(String inHex) {
        return (byte) Integer.parseInt(inHex, 16);
    }

    /**
     * 将ByteBuf转换为String
     * @param buf 待转换的ByteBuf数据
     * @return 转换后的String对象
     */
    public static String convertByteBufToString(ByteBuf buf) {
        String str;
        if(buf.hasArray()) { // 处理堆缓冲区
            str = new String(buf.array(), buf.arrayOffset() + buf.readerIndex(), buf.readableBytes());
        } else { // 处理直接缓冲区以及复合缓冲区
            byte[] bytes = new byte[buf.readableBytes()];
            buf.getBytes(buf.readerIndex(), bytes);
            str = new String(bytes, 0, buf.readableBytes());
        }
        return str;
    }
    /**
     * 将byte[]转换为String
     *
     * @return 转换后的String对象
     */
    public static String toHexString(byte[] byts) {
        StringBuilder sb = new StringBuilder();
        for (byte b : byts) {
            byte hi = (byte) ((b & 0xf0) >> 4);
            byte lo = (byte) (b & 0x0f);
            sb.append(hexchar(hi));
            sb.append(hexchar(lo));
        }
        return sb.toString();
    }

    public static char hexchar(byte val) {
        if (val >= 0 && val <= 9) {
            return (char) ('0' + val);
        } else if (val >= 10 && val <= 15) {
            return (char) ('A' + val - 10);
        } else {
            return ' ';
        }
    }

}
