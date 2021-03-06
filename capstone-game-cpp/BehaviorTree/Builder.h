﻿#pragma once
#include "Tree.h"
#include "Conditional.h"
#include "Predicate.h"

namespace BehaviorTree
{
	class Tree;

	class TreeBuilder
	{
		template <class ParentType, class CompositeType>
		class CompositeBuilder;

	public:
		template <class ParentType, class BehaviorType>
		class LeafBuilder
		{
			friend class TreeBuilder;

		public:
			LeafBuilder(TreeBuilder* root, ParentType* parent, UpdateCallback update, std::string name)
				: mRoot(root)
				, mParent(parent)
			{
				mBehavior = mRoot->Create<BehaviorType>(*mRoot->mTree, name);

				if (update)
				{
					mBehavior->SetUpdateCallback(update);
				}
			}

			template <typename ReturnType, typename ObjectType, typename... ParamTypes>
			LeafBuilder& Invoke(ReturnType(ObjectType::*method)(ParamTypes...), ParamTypes... params)
			{
				(mBehavior->*method)(params...);
				return (*this);
			}

			ParentType& End()
			{
				return *mParent;
			}

		private:
			TreeBuilder* mRoot;
			ParentType* mParent;
			BehaviorType* mBehavior;
		};

		template <class ParentType>
		class ConditionalBuilder
		{

		public:
			ConditionalBuilder(TreeBuilder* root, ParentType* parent, std::string name)
				: mRoot(root)
				, mParent(parent)
			{
				mConditional = mRoot->Create<Conditional>(*mRoot->mTree, nullptr, nullptr, name);
			}

			ConditionalBuilder<ParentType> Action(UpdateCallback update, std::string name = "Action")
			{
				auto builder = LeafBuilder<ConditionalBuilder, BehaviorTree::Behavior>(mRoot, this, update, name);
				mConditional->SetChild(builder.mBehavior);
				return *this;
			}

			template <class ChildCompositeType>
			CompositeBuilder<ConditionalBuilder<ParentType>, ChildCompositeType> Composite(std::string name = "Composite")
			{
				auto builder = CompositeBuilder<ConditionalBuilder<ParentType>, ChildCompositeType>(mRoot, this, name);
				mConditional->SetChild(builder.mComposite);
				return builder;
			}

			ConditionalBuilder<ParentType> Predicate(PredicateCallback predicate, std::string name = "Predicate")
			{
				auto builder = LeafBuilder<ConditionalBuilder, BehaviorTree::Predicate>(mRoot, this, nullptr, name);
				builder.Invoke(&Predicate::SetPredicateCallback, predicate);
				mConditional->SetPredicate(builder.mBehavior);
				return *this;
			}

			ParentType& End()
			{
				return *mParent;
			}

			TreeBuilder* mRoot;
			ParentType* mParent;
			Conditional* mConditional;
			std::string mName;
		};


		template <class ParentType, class DecoratorType>
		class DecoratorBuilder
		{
			friend class TreeBuilder;

		public:
			DecoratorBuilder(TreeBuilder* root, ParentType* parent, std::string name)
				: mRoot(root)
				, mParent(parent)
			{
				mDecorator = mRoot->Create<DecoratorType>(*mRoot->mTree, nullptr, name);
			}

			template <class ChildCompositeType>
			CompositeBuilder<DecoratorBuilder<ParentType, DecoratorType>, ChildCompositeType> Composite(std::string name = "Composite")
			{
				auto builder = CompositeBuilder<DecoratorBuilder<ParentType, DecoratorType>, ChildCompositeType>(mRoot, this, name);
				mDecorator->SetChild(builder.mComposite);
				return builder;
			}

			DecoratorBuilder<ParentType, DecoratorType> Action(UpdateCallback update, std::string name = "Action")
			{
				auto builder = LeafBuilder<DecoratorBuilder, BehaviorTree::Behavior>(mRoot, this, update, name);
				mDecorator->SetChild(builder.mBehavior);
				return *this;
			}

			DecoratorBuilder<ParentType, DecoratorType> Predicate(PredicateCallback predicate, std::string name = "Predicate")
			{
				auto builder = LeafBuilder<DecoratorBuilder, BehaviorTree::Predicate>(mRoot, this, nullptr, name);
				builder.Invoke(&Predicate::SetPredicateCallback, predicate);
				mDecorator->SetChild(builder.mBehavior);
				return *this;
			}

			ConditionalBuilder<DecoratorBuilder<ParentType, DecoratorType>> Conditional(std::string name = "Conditional")
			{
				auto builder = ConditionalBuilder<DecoratorBuilder>(mRoot, this, name);
				mDecorator->SetChild(builder.mConditional);
				return builder;
			}

			DecoratorBuilder<ParentType, DecoratorType> Subtree(Tree& tree, std::string name = "Subtree")
			{
				mDecorator->SetChild(tree);
				return *this;
			}

			ParentType& End()
			{
				return *mParent;
			}

		private:
			TreeBuilder* mRoot;
			ParentType* mParent;
			DecoratorType* mDecorator;
		};

		template <class ParentType, class CompositeType>
		class CompositeBuilder
		{
			friend class TreeBuilder;

		public:
			CompositeBuilder(TreeBuilder* root, ParentType* parent, std::string name)
				: mRoot(root)
				, mParent(parent)
			{
				mComposite = mRoot->Create<CompositeType>(*mRoot->mTree, name);
			}

			template <class NodeType, typename... ParamTypes>
			LeafBuilder<CompositeBuilder, NodeType> Node(ParamTypes... params);

			//template <typename... ParamTypes>
			//CompositeBuilder<CompositeBuilder<ParentType>> Composite(ParamTypes... params);

			template <class ChildCompositeType>
			CompositeBuilder<CompositeBuilder<ParentType, CompositeType>, ChildCompositeType> Composite(std::string name = "Composite")
			{
				auto builder = CompositeBuilder<CompositeBuilder<ParentType, CompositeType>, ChildCompositeType>(mRoot, this, name);
				mComposite->Add(*builder.mComposite);
				return builder;
			}

			CompositeBuilder<ParentType, CompositeType> Action(UpdateCallback update, std::string name = "Action")
			{
				auto builder = LeafBuilder<CompositeBuilder, BehaviorTree::Behavior>(mRoot, this, update, name);
				mComposite->Add(*builder.mBehavior);
				return *this;
			}

			CompositeBuilder<ParentType, CompositeType> Predicate(PredicateCallback predicate, std::string name = "Predicate")
			{
				auto builder = LeafBuilder<CompositeBuilder, BehaviorTree::Predicate>(mRoot, this, nullptr, name);
				builder.Invoke(&Predicate::SetPredicateCallback, predicate);
				mComposite->Add(*builder.mBehavior);
				return *this;
			}

			ConditionalBuilder<CompositeBuilder<ParentType, CompositeType>> Conditional(std::string name = "Conditional")
			{
				auto builder = ConditionalBuilder<CompositeBuilder>(mRoot, this, name);
				mComposite->Add(*builder.mConditional);
				return builder;
			}

			template <class DecoratorType>
			DecoratorBuilder<CompositeBuilder<ParentType, CompositeType>, DecoratorType> Decorator(std::string name = "Decorator")
			{
				auto builder = DecoratorBuilder<CompositeBuilder, DecoratorType>(mRoot, this, name);
				mComposite->Add(*builder.mDecorator);
				return builder;
			}

			CompositeBuilder<ParentType, CompositeType> Subtree(Tree& tree, std::string name = "Subtree")
			{
				mComposite->Add(tree);
				return *this;
			}

			ParentType& End()
			{
				return *mParent;
			}

		private:
			TreeBuilder* mRoot;
			ParentType* mParent;
			CompositeType* mComposite;
		};


		TreeBuilder(LinearAllocator& allocator, std::string name = "Tree")
			: mAllocator(allocator)
		{
			mTree = Create<Tree>(name);
			//mTree = new Tree(name);
		}

		template <class Type, typename... ArgTypes>
		Type* Create(ArgTypes... args)
		{
			void* mem = mAllocator.Allocate(sizeof(Type), alignof(Type), 0);
			return new (mem) Type(args...);
		}

		template <class NodeType>
		LeafBuilder<TreeBuilder, NodeType> Node(std::string name = "Node")
		{
			auto builder = LeafBuilder<TreeBuilder, NodeType>(this, this, nullptr, name);
			mTree->Start(*builder.mBehavior);
			return builder;
		}

		TreeBuilder Action(UpdateCallback update, std::string name = "Action")
		{
			auto builder = LeafBuilder<TreeBuilder, BehaviorTree::Behavior>(this, this, update, name);
			mTree->Start(*builder.mBehavior);
			return *this;
		}


		template <class CompositeType>
		CompositeBuilder<TreeBuilder, CompositeType> Composite(std::string name = "Composite")
		{
			auto builder = CompositeBuilder<TreeBuilder, CompositeType>(this, this, name);
			mTree->Start(*builder.mComposite);
			return builder;
		}

		//template <class DecoratorType, typename... ParamTypes>
		//DecoratorBuilder<TreeBuilder> Decorator(ParamTypes... params);


		Tree& End() const
		{
			return *mTree;
		}

	private:
		Tree* mTree;
		LinearAllocator& mAllocator;
	};
}
