/**

项目名称：StudyCase
类名称：LinkDemo
类描述：实现链表
创建人：Zoey
创建时间：2019年12月6日 下午2:30:17
@version
*/

package mylink;

interface ILink<E> {	//设置泛型避免安全隐患
	public void add(E e);	//增加数据
	public int size();	//获取数据的个数
	public boolean isEmpty();	//判断是否为空集合
	public Object [] toArray();	//将集合元素以数组的形式返回
	public E get(int index);	//根据索引获取数据
	public void set(int index, E data);		//修改索引数据
	public boolean contains(E data);		//判断数据是否存在
	public void remove(E data);		//数据删除
	public void clean();		//清空集合
}
//link类只负责数据的操作与根节点的处理，而所有后续节点都是node处理
class LinkImpl<E> implements ILink<E> {
	
	private class Node {	//保存节点的数据关系	内部类
		private E data;		//保存的数据
		private Node next;	//保存下一个引用
		public Node(E data) {	//有数据情况下才有意义
			this.data = data;
		}
		//第一次调用：this = LinkImpl.root;
		//第二次调用：this = LinkImpl.root.next;
		//第三次调用：this = LinkImpl.root.next.next;
		//总有next为空的时候
		public void addNode(Node newNode) {	//保存新的Node数据
			if (this.next == null) {	//当前节点的下一个节点为null
				this.next = newNode;	//保存当前节点
			} else {
				this.next.addNode(newNode);
			}
		}
		//第一次调用：this = LinkImpl.root;
		//第二次调用：this = LinkImpl.root.next;
		//第三次调用：this = LinkImpl.root.next.next;
		//递归获取数据
		public void toArrayNode() {
			LinkImpl.this.returnData [LinkImpl.this.foot++] = this.data;
			if (this.next != null) {	//还有下一个数据
				this.next.toArrayNode();
			}
			
		}
		public E getNode(int index) {
			if (LinkImpl.this.foot++ == index) {	//索引相同
				return this.data;	//返回当前数据
			} else {
				return this.next.getNode(index);
			}
		}
		public void setNode(int index, E data) {
			if (LinkImpl.this.foot++ == index) {	//索引相同
				this.data = data;	//修改数据
			} else {
				this.next.setNode(index, data);
			}
		}
		public boolean containsNode(E data) {
			if (this.data.equals(data)) {	//对象比较
				return true;
			} else {
				if (this.next == null) {	//没有后续节点了
					return false;	//找不到
				} else {
					return this.next.containsNode(data);	//向后继续判断
				}
			}
		}
		public void removeNode(Node previous, E data) {
			if (this.data.equals(data)) {
				previous.next = this.next;	//空出当前节点
			} else {
				if (this.next != null) {	//有后续节点
					this.next.removeNode(this, data);	//向后继续删除
				}
			}
		}
		
	}
	//------------------以下为link类中定义的成员-------------------------------
	
	private Node root; //保存根元素
	private int count;	//保存数据个数
	private int foot;	//描述的是操作数组的脚标
	private Object [] returnData;	//返回的数据保存
	
	//------------------以下为link类中定义的方法-------------------------------
	@Override
	public void add(E e) {
		// TODO 自动生成的方法存根
		if (e == null) {	//保存的数据为空
			return;		//方法调用直接结束
		} 
		//数据本身不具有关联特性的，只有Node类有，那么要想实现关联处理就必须将数据包装在Node类中
		Node newNode = new Node(e);	//创建一个新的节点
		if (this.root == null) {	//现在没有根节点
			this.root = newNode;	//第一个节点作为根节点			
		} else {				//根节点存在
			this.root.addNode(newNode);	//将新节点保存在合适的位置
		}	
		this.count++;
	}
	@Override
	public int size() {
		// TODO 自动生成的方法存根
		return this.count;
	}
	@Override
	public boolean isEmpty() {
		// TODO 自动生成的方法存根
		return this.count == 0;
//		return this.root == null;	一样
	}
	@Override
	public Object[] toArray() {
		// TODO 自动生成的方法存根
		if (this.isEmpty()) {	//空集合
			return null;	//没数据
		}
		this.foot = 0;
		//根据已有数据长度开辟数组
		this.returnData = new Object [this.count];
		this.root.toArrayNode(); 	//利用node类进行递归数据获取
		return returnData ;		
	}
	@Override
	public E get(int index) {
		// TODO 自动生成的方法存根
		if (index >= this.count) {	//索引应该在指定的范围内
			return null;
		}
		//索引数据的获取应该有node类完成
		this.foot = 0;	//重置索引下标
		return this.root.getNode(index);
	}
	@Override
	public void set(int index, E data) {
		// TODO 自动生成的方法存根
		if (index >= this.count) {	//索引应该在指定的范围内
			return;
		}
		//索引数据的获取应该有node类完成
		this.foot = 0;	//重置索引下标
		this.root.setNode(index, data);
	}
	@Override
	public boolean contains(E data) {
		// TODO 自动生成的方法存根
		if (data == null) {
			return false;	//没有数据
		}
		return this.root.containsNode(data);	//交给node判断
	}
	@Override
	public void remove(E data) {
		// TODO 自动生成的方法存根
		if (this.contains(data)) {	//判断数据是否存在
			if (this.root.data.equals(data)) {	//根节点为要删除的节点
				this.root = this.root.next;		//根的下一个节点
			} else {	//交由node类负责删除
				this.root.next.removeNode(this.root, data);
			}
			this.count--;
		}
	}
	@Override
	public void clean() {
		// TODO 自动生成的方法存根
		this.root = null;	//后续所有节点都没了
		this.count = 0;		//个数清零
	}
}

public class LinkDemo {

	public static void main(String[] args) {
		// TODO 自动生成的方法存根
		ILink<String> all = new LinkImpl<String>();
		System.out.println("【增加前】数据个数："+ all.size());
		all.add("hello");
		all.add("world");
		all.add("!");
		all.set(1, "世界");
		System.out.println("【增加后】数据个数："+ all.size());
		Object result [] = all.toArray();
		for (Object obj : result) {
			System.out.println(obj);
		}
		System.out.println("---------------------");
		System.out.println(all.get(2));
		System.out.println(all.get(5));
		System.out.println("---------------------");
		System.out.println(all.contains("Hello"));
		int i = 9 ;
        switch(i) {
            default:
                System.out.println("default");
            case 0 :
                System.out.println("zero");
                break ;
            case 1 : System.out.println("one");
            case 2 : System.out.println("two");
        }
	}

}
