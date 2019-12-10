package priv.zh.study.before;

class Person {
	private String name;
	private String addr;
	private char sex;
	private int age;

	public Person() {
	}

	public Person(String name, String addr) {
		this(name, addr, '男', 0);
	}

	public Person(String name, String addr, char sex, int age) {
		this.name = name;
		this.addr = addr;
		this.sex = sex;
		this.age = age;
	}

	public String getInfo() {
		return "姓名：" + this.name + "、地址：" + this.addr + "、性别：" + this.sex + "、年龄：" + this.age;
	}
}

class Student extends Person {
	private double math;
	private double english;

	public Student() {
	}

	public Student(String name, String addr, char sex) {
		super(name, addr);
	}

	public Student(String name, String addr, char sex, int age, double math, double english) {
		super(name, addr, sex, age);// 调用父类构造
		this.math = math;
		this.english = english;
	}

	@Override
	public String getInfo() {// 覆写父类方法
		return super.getInfo() + "、数学成绩：" + this.math + "、英语成绩：" + this.english;
	}
}

public class JavaExtends {

	public static void main(String[] args) {
		Student stu = new Student("张三", "天安门", '男', 12, 78.99, 89.98);
		System.out.println(stu.getInfo());
	}

}
