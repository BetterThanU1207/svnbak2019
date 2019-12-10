package generics.factory.factory;

public interface Factory<T> {
	T getProduct(Class<? extends T> clazz);
}
