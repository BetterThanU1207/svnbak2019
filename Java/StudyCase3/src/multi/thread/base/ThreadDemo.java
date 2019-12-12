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

	public static boolean flag = true;
	public static void main(String[] args) {
		// TODO 自动生成的方法存根
		new MyThread("线程A").start();
		new MyThread("线程B").start();
		new MyThread("线程C").start();
		
		//优雅停止线程		
		new Thread(() -> {
			long num = 0;
			while (flag) {
				try {
					Thread.sleep(50);
				} catch (InterruptedException e) {
					// TODO 自动生成的 catch 块
					e.printStackTrace();
				}
				System.out.println(Thread.currentThread().getName() + "正在运行、num = " + num++);
			}
		}, "执行线程").start();
		try {
			Thread.sleep(200);//运行200毫秒
		} catch (InterruptedException e) {
			// TODO 自动生成的 catch 块
			e.printStackTrace();
		}		
		flag = false;	//停止线程,解决线程停止最好用最方便的一种形式
	}
}
