// singleton_type.hpp
template <class DerivedT>
class singleton_type {
public:
	static DerivedT & get() {
		static DerivedT singleton{};
		return singleton;
	}

	bool is_singleton() const { return this == &get(); }
};