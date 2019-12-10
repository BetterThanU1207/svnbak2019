package priv.zh.study.before;

abstract class Action {
	public static final int EAT = 1;
	public static final int SLEEP = 5;
	public static final int WORK = 10;

	public void command(int code) {
		switch (code) {
		case EAT: {
			this.eat();
			break;
		}
		case SLEEP: {
			this.sleep();
			break;
		}
		case WORK: {
			this.work();
			break;
		}
		case EAT + SLEEP + WORK: {
			this.eat();
			this.sleep();
			this.work();
			break;
		}
		default:
			throw new IllegalArgumentException("Unexpected value: " + code);
		}
	}

	public abstract void eat();

	public abstract void sleep();

	public abstract void work();
}

class Robot extends Action {
	@Override
	public void eat() {
		// TODO 自动生成的方法存根
		System.out.println("机器人需要接通电源");
	}

	@Override
	public void sleep() {
		// TODO 自动生成的方法存根
	}

	@Override
	public void work() {
		// TODO 自动生成的方法存根
		System.out.println("机器人按照固定套路工作");
	}
}

class Human extends Action {
	@Override
	public void eat() {
		// TODO 自动生成的方法存根
		System.out.println("人吃饭");
	}

	@Override
	public void sleep() {
		// TODO 自动生成的方法存根
		System.out.println("人睡觉");
	}

	@Override
	public void work() {
		// TODO 自动生成的方法存根
		System.out.println("有想法的工作");
	}
}

class Pig extends Action {
	@Override
	public void eat() {
		// TODO 自动生成的方法存根
		System.out.println("猪吃饭");
	}

	@Override
	public void sleep() {
		// TODO 自动生成的方法存根
		System.out.println("猪睡觉");
	}

	@Override
	public void work() {

	}
}

public class JavaAbstract {

	public static void main(String[] args) {
		Action robotAction = new Robot();
		Action personAction = new Human();
		Action pigAction = new Pig();
		robotAction.command(Action.EAT);
		personAction.command(Action.EAT);
		pigAction.command(Action.EAT);
	}

}
