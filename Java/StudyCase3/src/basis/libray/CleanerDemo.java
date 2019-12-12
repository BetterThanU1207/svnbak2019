/**

项目名称：StudyCase3
类名称：Cleaner
类描述：
创建人：Zoey
创建时间：2019年12月10日 下午5:27:08
@version
*/

package basis.libray;

import java.lang.ref.Cleaner;

public class CleanerDemo {

	public static void main(String[] args) {
		// TODO 自动生成的方法存根
		try (MemberCleaning mc = new MemberCleaning()) {
			// 中间可以执行一些相关的代码
		} catch (Exception e) {
			// TODO: handle exception
		}
	}

}

class Member implements Runnable {
	public Member() {
		// TODO 自动生成的构造函数存根
		System.out.println("【构造】");
	}

	@Override
	public void run() { // 执行清除的时候执行的是此操作
		// TODO 自动生成的方法存根
		System.out.println("【回收】");
	}
}

class MemberCleaning implements AutoCloseable { // 实现清除处理
	private static final Cleaner cleaner = Cleaner.create(); // 创建一个清除处理
	private Member member;
	private Cleaner.Cleanable cleanable;
	
	public MemberCleaning() {
		// TODO 自动生成的构造函数存根
		this.member = new Member(); // 创建新对象
		this.cleanable = this.cleaner.register(this, this.member); // 注册使用的对象
	}

	@Override
	public void close() throws Exception {
		// TODO 自动生成的方法存根
		this.cleanable.clean(); // 启动多线程
	}
}