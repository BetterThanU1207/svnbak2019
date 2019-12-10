package multi.thread.demo;

class MyThread implements Runnable {
	private int ticket = 5;

	@Override
	public void run() {
		// TODO 自动生成的方法存根
		for (int i = 0; i < 100; i++) {
			if (this.ticket > 0) {
				System.out.println("卖票，ticket = ：" + this.ticket--);			
			}			
		}
	}
}

public class TicketDemo {

	public static void main(String[] args) {
		// TODO 自动生成的方法存根
		MyThread mt = new MyThread();//三个线程访问同一资源
		new Thread(mt).start();
		new Thread(mt).start();
		new Thread(mt).start();
 	}

}
