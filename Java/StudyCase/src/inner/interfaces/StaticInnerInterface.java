package inner.interfaces;

interface IMessageWarp {	//消息包装
	static interface IMessage {
		public String getContent();//消息内容
	}
	static interface IChannel {
		public boolean connect();//消息发送通道
	}
	public static void send(IMessage msg, IChannel channel) {
		if (channel.connect()) {
			System.out.println(msg.getContent());
		} else {
			System.out.println("消息通道无法建立，发送失败！");
		}
	}
}

class DefaultMessage implements IMessageWarp.IMessage {
	@Override
	public String getContent() {
		// TODO 自动生成的方法存根
		return "消息内容：www.baidu.com!";
	}
}

class NetChannel implements IMessageWarp.IChannel {
	@Override
	public boolean connect() {
		// TODO 自动生成的方法存根
		return true;
	}
}

public class StaticInnerInterface {

	public static void main(String[] args) {
		// TODO 自动生成的方法存根
		IMessageWarp.send(new DefaultMessage(), new NetChannel());
	}

}
