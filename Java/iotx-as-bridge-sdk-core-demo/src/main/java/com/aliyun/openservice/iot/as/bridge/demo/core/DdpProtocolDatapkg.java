package com.aliyun.openservice.iot.as.bridge.demo.core;

//import com.sun.org.apache.xml.internal.security.Init;

import com.aliyun.iot.as.bridge.core.handler.tsl.TslUplinkHandler;
import io.netty.buffer.ByteBuf;
import io.netty.buffer.Unpooled;
import io.netty.buffer.UnpooledDirectByteBuf;
import io.netty.channel.ChannelHandlerContext;
//import java.util.*;
import java.nio.charset.Charset;
import java.util.HashMap;
import java.util.Map;
import java.util.Random;

/**
 * Created by Administrator on 2019/11/8.
 * (宏电）DDP协议数据包类，数据帧格式定义：
 * -----------------------------------------------------------------------------------------------------------------
 *      起始标志（1B） |   包类型（1B) |   包长度（2B） |   DTU身份识别码（11B）  |   数据（0-1024B） |   结束标志（1B）
 * -----------------------------------------------------------------------------------------------------------------
 *      0x7B         |              |               |                       |                   |       0x7B
 * -----------------------------------------------------------------------------------------------------------------
 */
public class DdpProtocolDatapkg {
    private Byte headerTag;//起始标志
    private Byte tailTag;//结束标志

    private Byte pkgType;//包类型
    public Byte getPkgType(){return pkgType;}

    private Short pkgLength;//包长度
    public Short getPkgLength(){return pkgLength;}

    private String dtuIdentity;//DTU 身份识别码
    public  String getDtuIdentity(){return dtuIdentity;}

    private ByteBuf bodyMobus=null;//数据内容
    public int getIntFromPosition(int offset,boolean isLE)
    {
        if(isLE)
            return bodyMobus.getIntLE(offset);
        else
            return bodyMobus.getInt(offset);
    }

    public short getShortFromPosition(int offset,boolean isLE)
    {
        if(isLE)
            return bodyMobus.getShortLE(offset);
        else
            return bodyMobus.getShort(offset);
    }

    public long getLongFromPosition(int offset,boolean isLE)
    {
        if(isLE)
            return bodyMobus.getLongLE(offset);
        else
            return bodyMobus.getLong(offset);
    }

    public long getLongFromPositionIEEE754(int offset,boolean isLE)
    {
        byte[] data = new byte[4];
        bodyMobus.getBytes(offset,data,0,4);
        //2:LWHB
        byte[] tmp= new byte[2];
        tmp[0]=data[0];
        tmp[1]=data[1];
        data[0]=data[2];
        data[1]=data[3];
        data[2]=tmp[0];
        data[3]=tmp[1];

        ByteBuf bb = Unpooled.buffer(4,4);
        bb.writeBytes(data);

        return bb.readUnsignedInt();
        /*if(isLE)
            return bodyMobus.getLongLE(offset);
        else
            return bodyMobus.getLong(offset);*/
    }

    public float getFloatFromPosition(int offset,boolean isLE)
    {
        if(isLE)
            return bodyMobus.getFloatLE(offset);
        else
            return bodyMobus.getFloat(offset);
    }

    public float getFloatFromPositionIEEE754(int offset,boolean isLE)
    {
        byte[] data = new byte[4];
        bodyMobus.getBytes(offset,data,0,4);
        //2:LWHB
        byte[] tmp= new byte[2];
        tmp[0]=data[0];
        tmp[1]=data[1];
        data[0]=data[2];
        data[1]=data[3];
        data[2]=tmp[0];
        data[3]=tmp[1];

        ByteBuf bb = Unpooled.buffer(4,4);
        bb.writeBytes(data);

        return bb.readFloat();
    }

    public int getSlaveAddress(){
        //获取当前从站地址
        if(bodyMobus!=null){
            return (int)bodyMobus.getByte(0);
        }else
            return 0;
    }

    public static String[] cmdDesc={
            "请求注册或心跳","请求注销","","无效协议或命令","DSC用户数据应答包","","","","DSC用户数据包","","DTU参数查询应答包","","DTU参数设置应答包","DTU日志提取应答包","远程升级回应包"
    };

    //根据传入的ByteBuf构建DdpProtocolDatapkg对象
    public static DdpProtocolDatapkg newDdpProtocolDatapkg(ChannelHandlerContext ctx, ByteBuf in)
    {
        Byte header = in.readByte();
        Byte type = in.readByte();
        Short length = in.readShort();
        byte[] dtu = new byte[11];
        in.readBytes(dtu,0,11);
        String dtuid = new String(dtu);
        ByteBuf body = ctx.alloc().buffer(length-16);
        in.readBytes(body,length-16);
        Byte tail = in.readByte();

        in.resetReaderIndex();
        byte[] m = new byte[in.readableBytes()];
        in.readBytes(m);

        BridgeBasicV1.getLogger().warn("接收到来自主站[{}]的[{}]数据：{}",dtuid,cmdDesc[type-1],BridgeBasicV1.toHexString(m));
        return new DdpProtocolDatapkg(header,type,length,dtuid,body,tail);
    }

    public DdpProtocolDatapkg(Byte header,Byte type,Short length,String dtuId,ByteBuf buf,Byte tail)
    {
        headerTag = header;
        pkgType = type;
        pkgLength = length;
        dtuIdentity = dtuId;
        bodyMobus = buf;
        tailTag = tail;
    }

    //验证数据指令的有效性
    public String ValidationData(String exsitIdentity)
    {
        String ret = "";
        if((headerTag!=tailTag)||(headerTag!=0x7B))//判断包头和包尾
            String.format(ret,"无效的数据指令：起始标识{0}与结束标识{1}不一致",headerTag,tailTag);
        else if((pkgType!=0x01)&&(pkgType!=0x02)&&(pkgType!=0x04)&&(pkgType!=0x05)&&(pkgType!=0x09)&&(pkgType!=0x0B)&&(pkgType!=0x0D)&&(pkgType!=0x0E)&&(pkgType!=0x0F))//判断指令有效性
                String.format(ret,"无效的数据指令：无法识别的指令类别{0}",pkgType);
        else if((pkgType!=0x01)&&(!dtuIdentity.equalsIgnoreCase(exsitIdentity)))//判断主站ID
            String.format(ret,"无效的数据指令：接收的数据包中的主站标识{0}与当前注册的主站标识{1}不一致！",dtuIdentity,exsitIdentity);

        return ret;
    }

    //验证校验码
    public boolean CheckCRC(int type)
    {
        int dataLen= (int)bodyMobus.getByte(2)+3;//?????
        byte[] data = new byte[dataLen];
        bodyMobus.getBytes(0,data,0,dataLen);
        short crcCode = getCrc(data,type,false);
        if(crcCode == bodyMobus.getShort(dataLen))
            return true;
        else {
            BridgeBasicV1.getLogger().warn("校验错误：mobusData={}", bodyMobus.toString(Charset.defaultCharset()));
            return false;
        }
    }

    //获取指定字节的crc校验码
    public static short getCrc(byte[] buf,int type,boolean isOut)
    {
        byte[] b = Crc16Util.getCrc16(buf);
        return (short) (((b[0] << 8) | b[1] & 0xff));
    }

    //生成指令（NettyServer--->DTU)
    public static ByteBuf creatOrderToDtu(ChannelHandlerContext ctx,String dtuId,int order,TerminalsConfMgr.MasterBean.SlaveBean slave)
    {
        ByteBuf retBuf = null;
        //根据slave信息构建读取数据的指令
        switch (order)
        {
            case 19://0x13--通过远程唤醒DTU，不需要应答包
                break;
            case 20://0x14--短信通知启用DDP管理通道，不需要应答包
                break;
            case 129://0x81--注册成功
                retBuf = ctx.alloc().buffer(16);
                retBuf.writeByte((byte)123);//0x7B
                retBuf.writeByte((byte)129);//0x81
                retBuf.writeShort((short)16);//0x10
                retBuf.writeBytes(dtuId.getBytes());//DTu设备标志
                retBuf.writeByte((byte)123);//0x7B
                break;
            case 130://0x82--注销成功（DSC向DTU发送此命令时会让DTU重新启动）
                retBuf = ctx.alloc().buffer(16);
                retBuf.writeByte((byte)123);//0x7B
                retBuf.writeByte((byte)130);//0x82
                retBuf.writeShort((short)16);//0x10
                retBuf.writeBytes(dtuId.getBytes());//DTu设备标志
                retBuf.writeByte((byte)123);//0x7B
                break;
            case 131://0x83--DSC要求DTU向DSC重注册
                break;
            case 132://0x84--无效命令或协议包（一般不使用）
                retBuf = ctx.alloc().buffer(16);
                retBuf.writeByte((byte)123);//0x7B
                retBuf.writeByte((byte)132);//0x84
                retBuf.writeShort((short)16);//0x10
                retBuf.writeBytes(dtuId.getBytes());//DTu设备标志
                retBuf.writeByte((byte)123);//0x7B
                break;
            case 133://0x85--接收到DTU用户数据的应答包
                break;
            case 137://0x89--发送给DTU的用户数据包
                assert (slave!=null);
                //构建MobUS数据包
                ByteBuf body=null;//保存报文主体的字节数组
                int bodyLen = 8;//计算整个报文的长度
                body = ctx.alloc().buffer(bodyLen);
                body.writeByte(slave.getAddress());
                body.writeByte(3);
                if(slave.getType()==1){//jswater数据协议
                    body.writeShort(4113);//0x1010
                    body.writeShort(15);//0x000f
                }else{//dalian-haifeng数据协议
                    body.writeShort(0);//0x0000
                    body.writeShort(42);//0x002A
                }
                byte[] bodyArray = new byte[bodyLen-2];
                body.getBytes(0,bodyArray,0,bodyLen-2);
                body.writeShort(getCrc(bodyArray,1,true));
                //构建DDP数据包
                retBuf = ctx.alloc().buffer(bodyLen+16);
                retBuf.writeByte((byte)123);//0x7B
                retBuf.writeByte((byte)137);//0x89
                retBuf.writeShort((short)bodyLen);//mobus报文长度
                retBuf.writeBytes(dtuId.getBytes());//DTu设备标志
                retBuf.writeBytes(body);
                retBuf.writeByte((byte)123);//0x7B
                break;
            case 139://0x8B--查询DTU参数
                break;
            case 141://0x8D--设置DTU参数
                break;
            case 142://0x8E--提取DTU日志
                break;
            case 143://0x8F--DSC通知DTU远程升级的数据包
                break;
        }
        return retBuf;
    }

}
