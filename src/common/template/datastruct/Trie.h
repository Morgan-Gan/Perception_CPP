#pragma once
#include <iostream>
#include <cstring>
using namespace std;

//ʹ���ֵ����洢Ӣ�ĵ��ʣ�ʹ�õĽṹ��26���ֵ����������ֵ��ʵĴ�Сд
#include <cstring>
#include <iostream>

namespace common_template
{
	/* trie�Ľڵ����� */
	template <int Size> //SizeΪ�ַ����Ĵ�С
	struct trie_node
	{
		int freq; //��ǰ�ڵ��Ƿ������Ϊ�ַ����Ľ�β,�����freq>0,��������ظ����ʣ�freq��ʾ�õ��ʵĴ�Ƶ
		int node; //�ӽڵ�ĸ���
		trie_node *child[Size]; //ָ���ӽڵ�ָ��

								/* ���캯�� */
		trie_node() : freq(0), node(0) { memset(child, 0, sizeof(child)); }
	};

	/* trie */
	template <int Size, typename Index> //SizeΪ�ַ����Ĵ�С��IndexΪ�ַ����Ĺ�ϣ����
	class trie
	{
	public:
		/* �������ͱ��� */
		typedef trie_node<Size> node_type;
		typedef trie_node<Size>* link_type;

		/* ���캯�� */
		trie(Index i = Index()) : index(i) { }

		/* �������� */
		~trie() { clear(); }

		/* ��� */
		void clear()
		{
			clear_node(root);
			for (int i = 0; i < Size; ++i)
				root.child[i] = 0;
		}

		/* �����ַ��� */
		template <typename Iterator>
		void insert(Iterator begin, Iterator end)
		{
			link_type cur = &root; //��ǰ�ڵ�����Ϊ���ڵ�
			for (; begin != end; ++begin)
			{
				if (!cur->child[index[*begin]]) //����ǰ�ַ��Ҳ���ƥ�䣬���½��ڵ�
				{
					cur->child[index[*begin]] = new node_type;
					++cur->node; //��ǰ�ڵ���ӽڵ�����һ
				}
				cur = cur->child[index[*begin]]; //����ǰ�ڵ�����Ϊ��ǰ�ַ���Ӧ���ӽڵ�
			}
			(cur->freq)++; //���ô�����һ���ַ��Ľڵ�Ŀ���ֹ��־Ϊ��
		}

		/* �����ַ��������C����ַ��������ذ汾 */
		void insert(const char *str)
		{
			insert(str, str + strlen(str));
		}

		/* �����ַ������㷨�Ͳ������� */
		template <typename Iterator>
		int getfreq(Iterator begin, Iterator end)
		{
			link_type cur = &root;
			for (; begin != end; ++begin)
			{
				if (!cur->child[index[*begin]])
					return false;
				cur = cur->child[index[*begin]];
			}
			return cur->freq;
		}

		/* �����ַ��������C����ַ��������ذ汾 */
		bool find(const char *str)
		{
			int freq = getfreq(str, str + strlen(str));
			return freq > 0;
		}

		/* �����ַ���str�Ĵ�Ƶ*/
		int getfreq(const char* str)
		{
			return getfreq(str, str + strlen(str));
		}

		/* ɾ���ַ��� */
		template <typename Iterator>
		bool erase(Iterator begin, Iterator end)
		{
			bool result; //���ڴ���������
			erase_node(begin, end, root, result);
			return result;
		}

		/* ɾ���ַ��������C����ַ��������ذ汾 */
		bool erase(const char *str)
		{
			return erase(str, str + strlen(str));
		}

		/* ���ֵ�����������������е��� */
		template <typename Functor>
		void traverse(Functor execute = Functor())
		{
			char word[100] = { 0 };
			traverse_node(root, execute, word, 0);
		}

		/*����ֵ������ʵ��ܸ����������ظ��ַ���*/
		int sizeAll()
		{
			sizeAll(root);
		}

		int sizeAll(node_type& cur)
		{
			int size = cur.freq;
			for (int i = 0; i < Size; ++i)
			{
				if (cur.child[i] == 0)
					continue;
				size += sizeAll(*cur.child[i]);
			}
			return size;
		}

		/*����ֵ������ʵ��ܸ������ظ��ַ�����һ������*/
		int sizeNoneRedundant()
		{
			sizeNoneRedundant(root);
		}

		int sizeNoneRedundant(node_type& cur)
		{
			int size = cur.freq > 0 ? 1 : 0;
			for (int i = 0; i < Size; ++i)
			{
				if (cur.child[i] == 0)
					continue;
				size += sizeNoneRedundant(*cur.child[i]);
			}
			return size;
		}

		/*���ַ�����Ĺ���ǰ׺�ĳ���*/
		int maxPrefix_length()
		{
			int length = maxPrefix_length(root);
			return length - 1;   //��Ϊlength�����˸��ڵ㣬��Ҫɾ����
		}

		int maxPrefix_length(node_type& cur)
		{
			int length = 0;
			for (int i = 0; i < Size; ++i)
			{
				if (cur.child[i] != 0)
				{
					int tmp = maxPrefix_length(*cur.child[i]);
					if (tmp > length)
					{
						length = tmp;
					}

				}
			}
			if (length > 0 || cur.node > 1 || cur.freq > 0 && cur.node > 0)  //cur.node >1 ����"abcde"��"abcdf"���������cur.freq>0 && cur.node>0����"abcde"��"abcdef"�������
			{
				length++;
			}
			return length;

		}
		/*���ַ�������ǰ׺*/
		void maxPrefix(std::string& prefix)
		{
			maxPrefix(root, prefix);
			std::string word(prefix);
			int size = word.size();
			for (int i = 0; i < size; ++i)
				prefix[i] = word[size - 1 - i];
			prefix.erase(size - 1);   //��Ϊprefix�����˸��ڵ��ַ�����Ҫ����ɾ����

		}

		void maxPrefix(node_type& cur, std::string& prefix)
		{
			std::string word;
			int length = 0;
			int k = 0;
			for (int i = 0; i < Size; ++i)
			{
				if (cur.child[i] != 0)
				{
					maxPrefix(*cur.child[i], word);
					if (word.size() > length)
					{
						length = word.size();
						prefix.swap(word);
						k = i;
					}

				}
			}
			if (length > 0 || cur.node > 1 || cur.freq > 0 && cur.node > 0)  //cur.node >1 ����"abcde"��"abcdf"���������cur.freq>0 && cur.node>0����"abcde"��"abcdef"�������
			{
				prefix.push_back(k + 'a');
			}
		}

	private:

		template<typename Functor>
		void traverse_node(node_type& cur, Functor execute, char* word, int index)
		{
			if (cur.freq)
			{
				std::string str = word;
				execute(str, cur.freq);
			}
			for (int i = 0; i < Size; ++i)
			{
				if (cur.child[i] != 0)
				{
					word[index++] = 'a' + i;
					traverse_node(*cur.child[i], execute, word, index);
					word[index] = 0;
					index--;
				}
			}

		}


		/* ���ĳ���ڵ�������ӽڵ� */
		void clear_node(node_type& cur)
		{
			for (int i = 0; i < Size; ++i)
			{
				if (cur.child[i] == 0) continue;
				clear_node(*cur.child[i]);
				delete cur.child[i];
				cur.child[i] = 0;
				if (--cur.node == 0) break;
			}
		}

		/* ��������ɾ������ڵ㣬����ֵ�������丸�ڵ������Ƿ��ɾ���ýڵ� */
		template <typename Iterator>
		bool erase_node(Iterator begin, Iterator end, node_type &cur, bool &result)
		{
			if (begin == end) //�������ַ�����β���ݹ����ֹ����
			{
				result = (cur.freq > 0);   //�����ǰ�ڵ��Ƶ��>0,��ǰ�ڵ������Ϊ��ֹ�ַ�����ô���Ϊ��
				if (cur.freq)
					cur.freq--;            //�����ǰ�ڵ�Ϊ��ֹ�ַ�����Ƶ��һ
				return cur.freq == 0 && cur.node == 0;    //���ýڵ�Ϊ��Ҷ����ô֪ͨ�丸�ڵ�ɾ����
			}
			//���޷�ƥ�䵱ǰ�ַ�ʱ���������Ϊ�ٲ����ؼ٣���֪ͨ�丸�ڵ㲻Ҫɾ����
			if (cur.child[index[*begin]] == 0) return result = false;
			//�ж��Ƿ�Ӧ��ɾ�����ӽڵ�
			else if (erase_node((++begin)--, end, *(cur.child[index[*begin]]), result))
			{
				delete cur.child[index[*begin]]; //ɾ�����ӽڵ�
				cur.child[index[*begin]] = 0; //�ӽڵ�����һ
											//����ǰ�ڵ�Ϊ��Ҷ����ô֪ͨ�丸�ڵ�ɾ����
				if (--cur.node == 0 && cur.freq == 0) return true;
			}
			return false; //������������ؼ�
		}

		/* ���ڵ� */
		node_type root;

		/* ���ַ�ת��Ϊ������ת������������ */
		Index index;
	};

	//index function object
	class IndexClass
	{
	public:
		int operator[](const char key)
		{
			if (key >= 'a' && key <= 'z')
				return key - 'a';
			else if (key >= 'A' && key <= 'Z')
				return key - 'A';
		}
	};

	class StringExe
	{
	public:
		void operator()(std::string& str, int freq)
		{
			std::cout << str << ":" << freq << std::endl;
		}
	};
}
//int main()
//{
//	trie<26, IndexClass> t;
//	t.insert("tree");
//	t.insert("tree");
//	t.insert("tea");
//	t.insert("A");
//	t.insert("BABCDEGG");
//	t.insert("BABCDEFG");
//
//	t.traverse<StringExe>();
//
//	int sizeall = t.sizeAll();
//	std::cout << "sizeAll:" << sizeall << std::endl;
//
//	int size = t.sizeNoneRedundant();
//	std::cout << "size:" << size << std::endl;
//
//	std::string prefix;
//	int deep = t.maxPrefix_length();
//	t.maxPrefix(prefix);
//	std::cout << "deep:" << deep << " prefix:" << prefix << std::endl;
//
//	if (t.find("tree"))
//		std::cout << "find tree" << std::endl;
//	else
//		std::cout << "not find tree" << std::endl;
//
//	int freq = t.getfreq("tree");
//	std::cout << "tree freq:" << freq << std::endl;
//
//	if (t.erase("tree"))
//		std::cout << "delete tree" << std::endl;
//	else
//		std::cout << "not find tree" << std::endl;
//
//	freq = t.getfreq("tree");
//	std::cout << "tree freq:" << freq << std::endl;
//
//	if (t.erase("tree"))
//		std::cout << "delete tree" << std::endl;
//	else
//		std::cout << "not find tree" << std::endl;
//
//	if (t.erase("tree"))
//		std::cout << "delete tree" << std::endl;
//	else
//		std::cout << "not find tree" << std::endl;
//
//	sizeall = t.sizeAll();
//	std::cout << "sizeAll:" << sizeall << std::endl;
//
//	size = t.sizeNoneRedundant();
//	std::cout << "size:" << size << std::endl;
//
//
//	if (t.find("tre"))
//		std::cout << "find tre" << std::endl;
//	else
//		std::cout << "not find tre" << std::endl;
//
//
//
//	t.traverse<StringExe>();
//
//	return 0;
//}