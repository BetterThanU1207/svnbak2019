package priv.zh.study.before;

//数组操作类
class Array {
	private int[] data; // 整型数组
	private int foot; // 进行数组索引控制，int默认值为0

	public Array(int len) { // 数组长度外部定义
		if (len > 0) {
			this.data = new int[len]; // 开辟数组
		} else {
			this.data = new int[1]; // 开辟一个空间
		}
	}

	// 实现数组容量的扩充，给出的是扩充大小，实际大小=已有大小+扩充大小
	public void increment(int num) {
		int newData[] = new int[this.data.length + num];// 只能开辟新数组
		System.arraycopy(this.data, 0, newData, 0, this.data.length);
		this.data = newData; // 改变数组引用
	}

	public boolean add(int num) { // 数据增加
		if (this.foot < this.data.length) { // 有位置
			this.data[this.foot++] = num;
			return true;
		}
		return false;
	}

	public int[] getData() {
		return this.data;
	}
}

//排序子类
class SortArray extends Array {
	public SortArray(int len) {
		super(len);
	}

	@Override
	public int[] getData() { // 获取排序结果
		java.util.Arrays.sort(super.getData()); // 排序
		return super.getData();
	}
}

//反转子类
class ReverseArray extends Array {
	public ReverseArray(int len) {
		super(len);
	}

	@Override
	public int[] getData() {
		int center = super.getData().length / 2;
		int head = 0; // 头部角标
		int tail = super.getData().length - 1; // 尾部角标
		for (int x = 0; x < center; x++) {
			int temp = super.getData()[head];
			super.getData()[head] = super.getData()[tail];
			super.getData()[tail] = temp;
			head++;
			tail--;
		}
		return super.getData();
	}
}

public class JavaArray {

	public static void main(String[] args) {
		ReverseArray arr = new ReverseArray(5);
		System.out.println(arr.add(10));
		System.out.println(arr.add(5));
		System.out.println(arr.add(20));
		System.out.println(arr.add(3));
		System.out.println(arr.add(6));
		arr.increment(2);
		System.out.println(arr.add(1));
		System.out.println(arr.add(7));
		int result[] = arr.getData();
		for (int temp : result) {
			System.out.print(temp + "、");
		}
	}

}
