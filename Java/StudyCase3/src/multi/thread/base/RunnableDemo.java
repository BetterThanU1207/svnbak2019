package multi.thread.base;

class MyThread2 implements Runnable {
	private String title;
	public MyThread2(String title) {
		// TODO 自动生成的构造函数存根
		this.title = title;
	}
	@Override
	public void run() {
		// TODO 自动生成的方法存根
		for (int i = 0; i < 10; i++) {
			System.out.println("线程名称：" + this.title + "、运行次数：" + i);	
			System.out.println(Thread.currentThread().getName());
		}
	}
}

public class RunnableDemo {

	public static void main(String[] args) {
		// TODO 自动生成的方法存根
		Thread tA = new Thread(new MyThread2("A"), "线程A");
		Thread tB = new Thread(new MyThread2("B"));
		Thread tC = new Thread(new MyThread2("C"));
		tA.start();
		tB.start();
		tC.start();
	}

}
