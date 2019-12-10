/**

项目名称：StudyCase
类名称：FanXingDemo
类描述：泛型——实现坐标类
创建人：Zoey
创建时间：2019年12月4日 下午4:47:26
@version
*/

package priv.zh.study.improve;

class Point2<T> {
	private T x;
	private T y;
	public void setX(T x) {
		this.x = x;
	}
	public void setY(T y) {
		this.y = y;
	}
	public T getX() {
		return this.x;
	}
	public T getY() {
		return this.y;
	}
	
}

class Message<T> {
	private T content;
	public void setContent(T content) {
		this.content = content;
	}
	public T getContent() {
		return this.content;
	}
}

//	泛型接口
interface IMessage<T> {
	public String echo(T t);
}

//实现方式一：子类中继续使用泛型
class MessageImpl<S> implements IMessage<S> {
	@Override
	public String echo(S t) {
		// TODO 自动生成的方法存根
		return "【ECHO】" + t;
	}
}
//实现方式二：子类实现父接口时直接定义出具体的泛型类型
class MessageImpl2 implements IMessage<String> {
	@Override
	public String echo(String t) {
		// TODO 自动生成的方法存根
		return "【ECHO】" + t;
	}
}

public class FanXingDemo {

	public static void main(String[] args) {
		// TODO 自动生成的方法存根
		Point2<Integer> point = new Point2<Integer>();
		//	第一步 根据需求进行设置
		point.setX(10);
		point.setY(20);
		//	第二步 获取数据
		int x = point.getX();
		int y = point.getY();
		System.out.println("x坐标：" + x + "、y坐标：" + y);
		
		//通配符的使用，解决泛型引用传递
		Message<Integer> msgAMessage = new Message<Integer>();
		msgAMessage.setContent(25);
		fun(msgAMessage);
		Message<String> msgBMessage = new Message<String>();
		msgBMessage.setContent("I am a boy");
		fun(msgBMessage);
		
		//泛型接口
		IMessage<String> msg  = new MessageImpl<String>();
		System.out.println(msg.echo("JAVA."));
		IMessage<String> msg2  = new MessageImpl2();//不用在子类指定类型，但是父接口还要正常指定
		System.out.println(msg2.echo("JAVA."));
		
		//泛型方法使用
		Integer num [] = fun2(1, 2, 3);
		for (int temp : num) {
			System.out.print(temp + "、");			
		}
	}
	//	通配符不允许修改但是可以获取
	public static void fun(Message<?> temp) {
		System.out.println(temp.getContent());
	}
	
	//	泛型方法
	public static <T> T[] fun2(T ... args) {
		return args;
	}

}
