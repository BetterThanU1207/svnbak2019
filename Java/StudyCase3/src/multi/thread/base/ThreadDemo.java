package multi.thread.base;

class MyThread extends Thread {
	private String title;
	public MyThread(String title) {
		// TODO 自动生成的构造函数存根
		this.title = title;
	}
	@Override
	public void run() {
		// TODO 自动生成的方法存根
		for (int i = 0; i < 10; i++) {
			System.out.println("线程名称：" + this.title + "、运行次数：" + i);			
		}
	}
}

public class ThreadDemo {

	public static void main(String[] args) {
		// TODO 自动生成的方法存根
		new MyThread("线程A").start();
		new MyThread("线程B").start();
		new MyThread("线程C").start();
	}

}
