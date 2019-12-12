/**

项目名称：StudyCase3
类名称：Prod2Cons
类描述：生产者消费者案例，生产者生产消息，消费者等待生产者生产一条完整的消息进行消费，生产者等待消费者消费完一条消息再生产
创建人：Zoey
创建时间：2019年12月10日 上午9:41:48
@version	最原始处理方案，整个的等待、同步、唤醒机制都是开发者通过原生代码完成
*/

package multi.thread.cases;

class Message {
	private String title;
	private String content;
	private boolean flag = true;	//表示生产或消费的形式
	//flag=true：允许生产但是不允许消费
	//flag=false：不允许生产但是允许消费
	public synchronized void set(String title, String content) {
		if (this.flag == false) {	//无法进行生产，应该等待被消费
			try {
				super.wait();
			} catch (InterruptedException e) {
				// TODO 自动生成的 catch 块
				e.printStackTrace();
			}
		}
		this.title = title;
		try {
			Thread.sleep(100);
		} catch (InterruptedException e) {
			// TODO 自动生成的 catch 块
			e.printStackTrace();
		}
		this.content = content;
		this.flag = false;	//已经生产过了
		super.notify();		//唤醒等待的线程
	}
	
	public synchronized String get() {
		if (this.flag == true) {	//还未生产需要等待
			try {
				super.wait();
			} catch (InterruptedException e) {
				// TODO 自动生成的 catch 块
				e.printStackTrace();
			}
		}
		try {
			Thread.sleep(100);
		} catch (InterruptedException e) {
			// TODO 自动生成的 catch 块
			e.printStackTrace();
		}
		try {
			return this.title + "    --    " + this.content;
		} finally {		//不管如何都要执行
			// TODO: handle finally clause
			this.flag = true;	//继续生产
			super.notify(); 	//唤醒等待线程
		}
	}
}

public class Prod2Cons {

	public static void main(String[] args) {
		// TODO 自动生成的方法存根
		Message msg = new Message();
		new Thread(new Producer(msg)).start();	//启动生产者线程
		new Thread(new Consumer(msg)).start();	//启动消费者线程
	}

}
//生产者线程
class Producer implements Runnable {
	private Message msg;

	public Producer(Message msg) {
		// TODO 自动生成的构造函数存根
		this.msg = msg;
	}
	@Override
	public void run() {
		// TODO 自动生成的方法存根
		for (int i = 0; i < 100; i++) {
			if (i % 2 == 0) {
				this.msg.set("王建", "宇宙大帅哥。");
			} else {
				this.msg.set("小高", "猥琐第一人。");
			}			
		}
	}
}
//消费者线程
class Consumer implements Runnable {
	private Message msg;

	public Consumer(Message msg) {
		// TODO 自动生成的构造函数存根
		this.msg = msg;
	}
	@Override
	public void run() {
		// TODO 自动生成的方法存根
		for (int i = 0; i < 100; i++) {
			System.out.println(this.msg.get());			
		}
	}
}
