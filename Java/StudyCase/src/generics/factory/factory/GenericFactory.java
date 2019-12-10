package generics.factory.factory;

public class GenericFactory<T> implements Factory<T> {

	@Override
	public T getProduct(Class<? extends T> clazz) {
		// TODO 自动生成的方法存根
		if (null == clazz) {
			return null;
		}
		try {
//			@SuppressWarnings("deprecation")
			T objT = (T) clazz.newInstance();
			return objT;
		} catch (InstantiationException e) {
			// TODO: handle exception
			e.printStackTrace();
		}catch (IllegalAccessException e) {
			// TODO: handle exception
			e.printStackTrace();
		}
		return null;
	}
}
