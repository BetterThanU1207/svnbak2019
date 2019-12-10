package com.aliyun.openservice.iot.as.bridge.demo.core;

import com.aliyun.iot.as.bridge.core.model.Session;
//import com.sun.corba.se.impl.orbutil.closure.Future;
import io.netty.buffer.ByteBuf;
import io.netty.channel.Channel;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.util.ReferenceCountUtil;

import java.io.UnsupportedEncodingException;
import java.util.*;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

import com.aliyun.iot.as.bridge.core.handler.tsl.TslUplinkHandler;

/**
 * Created by Administrator on 2019/11/5.
 */
public class AdapterServerHandler extends ChannelInboundHandlerAdapter{

    private final static String TOPIC_TEMPLATE_USER_DEFINE = "/sys/%s/%s/thing/event/property/post";///sys/%s/%s/thing/model/up_raw

    //保存已建立的连接通道对象
    public static List<Channel> channels = new ArrayList<>();
    //生成上传数据到IoT的消息ID
    private final static Random random = new Random();

    private String masterIdentity="";//建立该连接的主站身份标识
    private List<TerminalsConfMgr.MasterBean.SlaveBean> slaves;//保存当前主站下所有从站设备信息
    private String remoteIp="";//主站移动IP

    //ScheduledFuture<?>
    private List<ScheduledFuture<?>> scheduledFutures = new ArrayList<>();

    //生成上传数据到IoT的消息ID
 //   private final static Random random = new Random();

    //清理现场，关闭设备通道
    private void ClearChanel(Channel in,ChannelHandlerContext ctx){
        //清理现场，关闭当前通道
        channels.remove(in);//移除管道对象
        //关闭各从站的计划任务
        for(ScheduledFuture<?> f:scheduledFutures)
        {
            f.cancel(false);
        }
        //清空Future列表
        scheduledFutures.clear();
        //关闭通道
        ctx.close();

        BridgeBasicV1.getLogger().warn("与主站【{}】的连接已关闭", masterIdentity);
    }

    /*
    根据设备连接请求获取该设备的原始身份标志
    * */
    private String getOriginalIdentity(String masteId,String slaveId){
        //String originalId="test";//默认为测试设备信息
        return masteId+slaveId;
    }

    @Override  //通道就绪---上线
    public void channelActive(ChannelHandlerContext ctx)  {
        Channel inChannel=ctx.channel();
        channels.add(inChannel);
    }
    @Override  //通道未就绪----离线
    public void channelInactive(ChannelHandlerContext ctx)  {
        Channel inChannel=ctx.channel();
        channels.remove(inChannel);
    }

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) throws UnsupportedEncodingException {

        //根据接收的字节数据构建DdpProtocolDatapkg对象
        DdpProtocolDatapkg data = DdpProtocolDatapkg.newDdpProtocolDatapkg(ctx, (ByteBuf) msg);
        //获取当前通道对象
        Channel inChannel = ctx.channel();
        /*
        //ByteBuf buf = (ByteBuf)msg;
        //byte[] req = new byte[buf.readableBytes()]；
        //buf.readBytes(req);
        //String body = new String(req,"UTF-8");

        ByteBuf m = (ByteBuf) msg;
        byte[] in = new byte[m.readableBytes()];
        m.readBytes(in);
        String body = new String(in);//,"UTF-8"
        ((ByteBuf) msg).resetReaderIndex();*/
        try {
            String ret = data.ValidationData(masterIdentity);
            if (ret != "") {//无效的指令
                BridgeBasicV1.getLogger().warn("{}", ret);
                //发送无效指令的应答
                if(masterIdentity!="")
                    sendToDTU(null, ctx, 132,masterIdentity);//0x84
            } else {
                //判断指令类型
                switch (data.getPkgType()) {
                    case 1://终端请求注册
                    {
                        if(masterIdentity=="") {
                            //获取当前主站身份标识（11Byte)
                            String masterID = data.getDtuIdentity();
                            //获取当前主站下的所有从站信息,为每个从站配置计划任务
                            slaves = BridgeBasicV1.terminalsConfMgr.getSlavesByMaster(masterID);
                            if(slaves!=null) {
                                //保存主站身份标志
                                masterIdentity = masterID;
                                //保存主站远程IP
                                remoteIp = inChannel.remoteAddress().toString().substring(1);
                                for (TerminalsConfMgr.MasterBean.SlaveBean s : slaves) {
                                    int interval = s.getInterval();
                                    //ScheduledFuture<?> future = inChannel.eventLoop().scheduleAtFixedRate(
                                    ScheduledFuture<?> future = ctx.executor().scheduleAtFixedRate(
                                            new Runnable() {
                                                @Override
                                                public void run() {
                                                    try {
                                                        //构建读取当前从站数据的指令信息
                                                        sendToDTU(s, ctx, 137, masterIdentity);//0x89
                                                    } catch (Exception e) {
                                                        e.printStackTrace();
                                                        BridgeBasicV1.getLogger().warn("执行计划任务失败（主站{}，从站{}）：{}",masterIdentity,s.getSlaveId(),e.getMessage());
                                                    }
                                                }
                                            }, 0,interval, TimeUnit.SECONDS);
                                    //保存计划任务返回的Future对象
                                    scheduledFutures.add(future);
                                }
                                //发送注册成功应答
                                sendToDTU(null, ctx, 129, masterIdentity);//0x81
                            }else{//没有查找到指定的主站信息
                                BridgeBasicV1.getLogger().warn("没有找到指定主站信息{},请检查设备配置文件是否正确！", masterID);
                                //清理现场，关闭当前通道
                                ClearChanel(inChannel, ctx);
                            }
                        }else {
                            //发送注册成功应答
                            sendToDTU(null, ctx, 129, masterIdentity);//0x81
                        }
                        break;
                    }
                    case 2://终端请求注销
                        //发送注销应答
                        sendToDTU(null, ctx,130,masterIdentity);//0x82
                        //清理现场，关闭当前通道
                        ClearChanel(inChannel, ctx);
                        break;
                    case 4://无效的命令或协议包（一般在查询或设置指令时使用）
                        BridgeBasicV1.getLogger().warn("设备【{}】接收到无效的命令或协议包", remoteIp);
                        break;
                    case 5://接收到dsc用户数据的应答包
                        break;
                    case 9://发送给dsc的用户数据包
                        int slaveAddr = data.getSlaveAddress();
                        if(slaveAddr>0) {
                            TerminalsConfMgr.MasterBean.SlaveBean slave =getSlaveIdentity(slaveAddr);
                            if(slave!=null) {
                                //验证数据校验码
                                if (data.CheckCRC(slave.getType())) {
                                    //original device identity, defined in devices.conf
                                    String originalIdentity = getOriginalIdentity(masterIdentity, slave.getSlaveId()); //获取原始身份标志
                                    //create session
                                    Session session = Session.newInstance(originalIdentity, inChannel);
                                    //device online
                                    TslUplinkHandler tslUplinkHandler = new TslUplinkHandler();
                                    if (!tslUplinkHandler.doOnline(session, originalIdentity)) {
                                        BridgeBasicV1.getLogger().warn("设备【{}】接入物联网平台失败...", remoteIp);
                                    } else {
                                        //推送数据到IoT平台------------data.publishToIOT(tslUplinkHandler, originalIdentity);
                                        String requestId = String.valueOf(random.nextInt(1000));
                                        Map<String, Object> properties = new HashMap<String, Object>();
                                        //添加参数信息到属性列表
                                        if (slave.getType() == 1) {//js-water
                                            //读取
                                            int value1 = data.getIntFromPosition(15,false);
                                            int value2 = data.getIntFromPosition(19,false);
                                            short value3 = data.getShortFromPosition(27,false);

                                            properties.put("IntegratedFlow", (float) value1 / 10);//累计流量

                                            int dn = slave.getDn();
                                            if (dn < 150)
                                                properties.put("InstantaneousFlow", (float) value2 / 100);
                                            else
                                                properties.put("InstantaneousFlow", (float) value2 / 10);

                                            properties.put("Pressure", value3);
                                        } else if (slave.getType() == 2) {//dalian-haifeng
                                            //读取
                                            float value1 = data.getFloatFromPositionIEEE754(3,true);
                                            long value2 = data.getLongFromPositionIEEE754(19,true);
                                            float value3 = data.getFloatFromPositionIEEE754(83,true);

                                            properties.put("InstantaneousFlow", value1);
                                            properties.put("ForwardIntegratedFlow", value2);
                                            properties.put("Pressure", value3);
                                        }
                                        //推送数据到IOT
                                        tslUplinkHandler.reportProperties(requestId, originalIdentity, properties);
                                        //断开到IoT的连接
                                        tslUplinkHandler.doOffline(originalIdentity);
                                        //推送数据完成
                                        BridgeBasicV1.getLogger().warn("设备【{}】成功推送数据到物联网平台...", remoteIp);
                                    }
                                }
                            }else {
                                BridgeBasicV1.getLogger().warn("未从主站{}下查找到地址为{}的从站设备...", masterIdentity,slaveAddr);
                            }
                        }else{
                            BridgeBasicV1.getLogger().warn("从站设备地址无效（主站：{}）...", masterIdentity);
                        }
                        break;
                    case 11://查询DTU参数的应答包
                        break;
                    case 13://设置DTU参数的应答包
                        break;
                    case 14://提取DTU日志的应答包
                        break;
                    case 15://远程升级的回应包
                        break;
                }
            }
        } catch (Exception err) {
            BridgeBasicV1.getLogger().warn("channelRead出现异常：{}", err.getMessage());
        } finally {
            ReferenceCountUtil.release(msg);
        }
    }

    //获取当前slaveid
    private TerminalsConfMgr.MasterBean.SlaveBean getSlaveIdentity(int address)
    {
        for(TerminalsConfMgr.MasterBean.SlaveBean s:slaves)
        {
            if(s.getAddress()==address){
                return s;
            }
        }
        return null;
    }

    private static String sendCmd[]={"注册成功","注销成功","要求DTU重新注册","无效命令","接收DTU用户数据的应答包","","","","发送给DTU的用户数据包","","查询DTU参数","","设置DTU参数","提取DTU日志","通知DTU远程升级的数据包"};
    //向DTU发送数据
    private void sendToDTU(TerminalsConfMgr.MasterBean.SlaveBean s, ChannelHandlerContext ctx,int order,String dtuId) {
        ByteBuf out = DdpProtocolDatapkg.creatOrderToDtu(ctx,dtuId,order,s);
        if(out!=null) {
            byte[] m = new byte[out.readableBytes()];
            out.readBytes(m);
            out.resetReaderIndex();
            ctx.writeAndFlush(out);//发送读取数据的指令
            BridgeBasicV1.getLogger().warn("向主站[{}]发送的[{}]数据：{}",dtuId,sendCmd[order-129],BridgeBasicV1.toHexString(m));
        }
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        // 当出现异常就关闭连接
        Channel inChannel=ctx.channel();
        //cause.printStackTrace();
        BridgeBasicV1.getLogger().warn("设备【{}】异常：{}", remoteIp,cause.getMessage());
        //清理现场，关闭当前通道
        ClearChanel(inChannel,ctx);
    }
}
