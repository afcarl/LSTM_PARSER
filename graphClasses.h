//creating the structure of the nn in a graph that will help in performing backpropagation and forward propagation
#pragma once

#include <cstdlib>
#include "neuralClasses.h"
#include <Eigen/Dense>
#include <Eigen/Core>
#include <Eigen/Eigen>


using namespace Eigen;
namespace nplm
{


using namespace Eigen;
	
	struct stateClipper{
	  precision_type operator() (precision_type x) const { 
	    return (precision_type) std::min(25., std::max(double(x),-25.));
	    //return(x);
	  }
	};

typedef Matrix<int,Dynamic,Dynamic> IndexMatrix;
typedef Matrix<precision_type,Dynamic,Dynamic> DoubleMatrix;



template <class X>
class Node {
    public:
        X * param; //what parameter is this
        //vector <void *> children;
        //vector <void *> parents;
		//typedef typename X::fProp_data fProp_data;
		//typedef typename X::bProp_data bProp_data;
		//fProp_data fProp_matrix;
		//bProp_data bProp_maxtrix;
	Eigen::Matrix<precision_type,Eigen::Dynamic,Eigen::Dynamic> fProp_matrix;
	Eigen::Matrix<precision_type,Eigen::Dynamic,Eigen::Dynamic> bProp_matrix;
	int minibatch_size;

    public:
		/*
		fprop(inmatrix)
		{
			param->fprop(inmatrix,fProp_data);
		}
		*/
        Node() : param(NULL), minibatch_size(0) { }

        Node(X *input_param, int minibatch_size)
	  : param(input_param),
	    minibatch_size(minibatch_size)
        {
	    resize(minibatch_size);
        }

	void resize(int minibatch_size)
	{
	    this->minibatch_size = minibatch_size;
	    if (param->n_outputs() != -1)
	    {
	        fProp_matrix.setZero(param->n_outputs(), minibatch_size);
	    }
        if (param->n_inputs() != -1)
        {
        	bProp_matrix.setZero(param->n_inputs(), minibatch_size);
        }
	}

	void resize() { resize(minibatch_size); }

};

class Output_loss_node {
	int minibatch_size;
public:
	Matrix<precision_type,Dynamic,Dynamic> d_Err_t_d_h_t;
	Output_loss_node() :minibatch_size(0),d_Err_t_d_h_t(Matrix<precision_type,Dynamic,Dynamic>()) {}
	
	void resize(int num_hidden,int minibatch_size) {
		//d_Err_t_d_h_t.resize(num_hidden, minibatch_size);
		//Need to make this smarter
		this->minibatch_size = minibatch_size;
		d_Err_t_d_h_t.setZero(num_hidden, minibatch_size);
	}
	//void resize(int num_hidden,int minibatch_size){
	//	d_Err_t_d_h_t.resize(num_hidden, minibatch_size);
	//}
};

template <class input_node_type>
class LSTM_node {
	int minibatch_size;
public:
	//Each LSTM node has a bunch of nodes and temporary data structures
    //Node<Input_word_embeddings> input_layer_node,W_x_to_i_node, W_x_to_f_node, W_x_to_c_node, W_x_to_o_node;
    //Node<Linear_layer> W_x_to_i_node, W_x_to_f_node, W_x_to_c_node, W_x_to_o_node;
	Node<Linear_layer> W_h_to_i_node, W_h_to_f_node, W_h_to_c_node, W_h_to_o_node;
	Node<Linear_diagonal_layer> W_c_to_i_node, W_c_to_f_node, W_c_to_o_node;
    Node<Hidden_layer> i_t_node,f_t_node,o_t_node,tanh_c_prime_t_node;
	Node<Activation_function> tanh_c_t_node;
	Dropout_layer h_t_dropout_layer;
	

	Eigen::Matrix<precision_type,Eigen::Dynamic,Eigen::Dynamic> h_t,c_t,c_t_minus_one, h_t_minus_one;
	Eigen::Matrix<precision_type,Eigen::Dynamic,Eigen::Dynamic> d_Err_t_to_n_d_h_t,
														d_Err_t_to_n_d_c_t,
														d_Err_t_to_n_d_o_t,
														d_Err_t_to_n_d_f_t,
														d_Err_t_to_n_d_i_t,
														d_Err_t_to_n_d_tanh_c_t,
														d_Err_t_to_n_d_tanh_c_prime_t,
														d_Err_t_to_n_d_x_t,
														i_t_input_matrix,
														f_t_input_matrix,
														o_t_input_matrix,
														tanh_c_prime_t_input_matrix;
														//tanh_c_t_input_matrix;
														
														
	Eigen::Matrix<precision_type,Eigen::Dynamic,Eigen::Dynamic>	d_Err_t_to_n_d_h_tMinusOne,
														d_Err_t_to_n_d_c_tMinusOne;
										
	input_node_type *input_node;
	
	LSTM_node(): 

		W_h_to_i_node(),
		W_h_to_f_node(),
		W_h_to_c_node(),
		W_h_to_o_node(),
		W_c_to_i_node(),
		W_c_to_f_node(),
		W_c_to_o_node(),
		i_t_node(),
		f_t_node(),
		o_t_node(),
		tanh_c_prime_t_node(),
		tanh_c_t_node(),
		h_t_dropout_layer(Dropout_layer()),
		//input_layer_node(),
		input_node(NULL) {}

	LSTM_node(model &lstm, int minibatch_size): 
		W_h_to_i_node(&lstm.W_h_to_i, minibatch_size),
		W_h_to_f_node(&lstm.W_h_to_f, minibatch_size),
		W_h_to_c_node(&lstm.W_h_to_c, minibatch_size),
		W_h_to_o_node(&lstm.W_h_to_o, minibatch_size),
		W_c_to_i_node(&lstm.W_c_to_i, minibatch_size),
		W_c_to_f_node(&lstm.W_c_to_f, minibatch_size),
		W_c_to_o_node(&lstm.W_c_to_o, minibatch_size),
		i_t_node(&lstm.i_t,minibatch_size),
		f_t_node(&lstm.f_t,minibatch_size),
		o_t_node(&lstm.o_t,minibatch_size),
		tanh_c_prime_t_node(&lstm.tanh_c_prime_t,minibatch_size),
		tanh_c_t_node(&lstm.tanh_c_t,minibatch_size),
		//input_layer_node(&lstm.input_layer,minibatch_size),
		input_node(NULL)
		 {
			this->minibatch_size = minibatch_size;
		 }
	//Resizing all the parameters
	void resize(int minibatch_size){
		this->minibatch_size = minibatch_size;
		W_h_to_i_node.resize(minibatch_size);
		W_h_to_f_node.resize(minibatch_size);
		W_h_to_c_node.resize(minibatch_size);
		W_h_to_o_node.resize(minibatch_size);
		W_c_to_i_node.resize(minibatch_size);
		W_c_to_f_node.resize(minibatch_size);
		W_c_to_o_node.resize(minibatch_size);
		i_t_node.resize(minibatch_size);
		f_t_node.resize(minibatch_size);
		o_t_node.resize(minibatch_size);
		tanh_c_prime_t_node.resize(minibatch_size);
		//input_layer_node.resize(minibatch_size);
		
		//Resizing all the local node matrices
		h_t.setZero(W_h_to_i_node.param->n_inputs(),minibatch_size);
		c_t.setZero(W_c_to_i_node.param->n_inputs(),minibatch_size);
		//h_t_minus_one.setZero(W_h_to_i_node.param->n_inputs(),minibatch_size);
		//c_t_minus_one.setZero(W_c_to_i_node.param->n_inputs(),minibatch_size);
		//cerr<<"c_t_minus_one.rows() "<<c_t_minus_one.rows()<<" c_t_minus_one.cols() "<<c_t_minus_one.cols()<<endl;
		d_Err_t_to_n_d_h_t.setZero(W_h_to_i_node.param->n_outputs(),minibatch_size);
		d_Err_t_to_n_d_c_t.setZero(W_c_to_i_node.param->n_outputs(),minibatch_size);
		d_Err_t_to_n_d_o_t.setZero(o_t_node.param->n_outputs(),minibatch_size);
		d_Err_t_to_n_d_f_t.setZero(f_t_node.param->n_outputs(),minibatch_size);
		d_Err_t_to_n_d_i_t.setZero(i_t_node.param->n_outputs(),minibatch_size);
		d_Err_t_to_n_d_tanh_c_t.setZero(tanh_c_t_node.param->n_outputs(),minibatch_size);
		d_Err_t_to_n_d_tanh_c_prime_t.setZero(tanh_c_prime_t_node.param->n_outputs(),minibatch_size);
		//d_Err_t_to_n_d_x_t.setZero(input_layer_node.param->n_outputs(),minibatch_size);
		d_Err_t_to_n_d_h_tMinusOne.setZero(W_h_to_i_node.param->n_outputs(),minibatch_size);
		d_Err_t_to_n_d_c_tMinusOne.setZero(W_c_to_i_node.param->n_outputs(),minibatch_size);
		i_t_input_matrix.setZero(i_t_node.param->n_inputs(),minibatch_size);
		f_t_input_matrix.setZero(f_t_node.param->n_inputs(),minibatch_size);
		o_t_input_matrix.setZero(o_t_node.param->n_inputs(),minibatch_size);
		tanh_c_prime_t_input_matrix.setZero(tanh_c_prime_t_node.param->n_inputs(),minibatch_size);
		
	} 
	void resizeDropout(int minibatch_size,
						precision_type dropout_probability){
		resize (minibatch_size);
		//If using dropout
		//cerr<<"In resize dropout "<<endl;
		//getchar();
		if (dropout_probability > 0) {
			//cerr<<"ht cols is "<<this->h_t.rows()<<endl;
			h_t_dropout_layer = Dropout_layer(this->h_t.rows(), 
											minibatch_size, 
											1.-dropout_probability);
		} else {
			//cerr<<"Warning the dropout probability has to be greater than 0!"<<endl;
			//exit(1);
		}			
	}

	void set_input_node(input_node_type &input_node){this->input_node = &input_node;}

	template<typename Derived> //, typename DerivedCIn, typename DerivedHIn>
	void fPropInput(const MatrixBase<Derived> &data) {
		input_node->fProp(data);
	}
	template<typename Derived, typename Engine>
	void fPropInputDropout(const MatrixBase<Derived> &data,
					  Engine &eng) {
		input_node->fPropDropout(data,eng);
	}		
	template<typename DerivedData, typename DerivedIn, typename DerivedDCIn, typename DerivedDHIn, typename DerivedH, typename DerivedC>
	void bProp(const MatrixBase<DerivedData> &data,
			   const MatrixBase<DerivedIn> &d_Err_t_d_h_t,
			   const MatrixBase<DerivedDCIn> &d_Err_tPlusOne_to_n_d_c_t,
			   const MatrixBase<DerivedDHIn> &d_Err_tPlusOne_to_n_d_h_t,
			   const MatrixBase<DerivedH> &h_t_minus_one,
			   const MatrixBase<DerivedC> &c_t_minus_one,
			   bool gradient_check,
			   bool norm_clipping){
				   
		int current_minibatch_size = data.cols();
		
		bPropLSTMBlock(d_Err_t_d_h_t,
				   d_Err_tPlusOne_to_n_d_c_t,
				   d_Err_tPlusOne_to_n_d_h_t,
				   h_t_minus_one,
				   c_t_minus_one,
				   gradient_check,
				   norm_clipping,
				   current_minibatch_size);	
				   		
		bPropInput(data);	
	}
	
	template<typename DerivedData, typename DerivedIn, typename DerivedDCIn, typename DerivedDHIn, typename DerivedH, typename DerivedC>
	void bPropDropout(const MatrixBase<DerivedData> &data,
			   const MatrixBase<DerivedIn> &d_Err_t_d_h_t,
			   const MatrixBase<DerivedDCIn> &d_Err_tPlusOne_to_n_d_c_t,
			   const MatrixBase<DerivedDHIn> &d_Err_tPlusOne_to_n_d_h_t,
			   const MatrixBase<DerivedH> &h_t_minus_one,
			   const MatrixBase<DerivedC> &c_t_minus_one,
			   bool gradient_check,
			   bool norm_clipping){
	   		int current_minibatch_size = data.cols();
			//Make sure the dropout is applied to both the error from time 
			//t and time t+1 to n1 and not just d_Err_t_d_h_t because the dropped out h_t
			//is used for both predicting the output label and for the next
			//LSTM block
			h_t_dropout_layer.bProp(d_Err_t_d_h_t);
			h_t_dropout_layer.bProp(d_Err_tPlusOne_to_n_d_h_t);
			//cerr<<"c_t_minus_one.rows() "<<c_t_minus_one.rows()<<" c_t_minus_one.cols() "<<c_t_minus_one.cols()<<endl;
	   		bPropLSTMBlock(d_Err_t_d_h_t,
	   				   d_Err_tPlusOne_to_n_d_c_t,
	   				   d_Err_tPlusOne_to_n_d_h_t,
					   h_t_minus_one,
					   c_t_minus_one,
	   				   gradient_check,
	   				   norm_clipping,
	   				   current_minibatch_size);	
			   		
	   		bPropInputDropout(data);			
	}

	template<typename Derived, typename DerivedH, typename DerivedC>
    void fProp(const MatrixBase<Derived> &data,
			   const MatrixBase<DerivedH> &h_t_minus_one,
			   const MatrixBase<DerivedC> &c_t_minus_one) { //,	

		fPropInput(data);
		fPropLSTMBlock(h_t_minus_one,
					   c_t_minus_one);


	}
	
	template<typename Derived, typename Engine, typename DerivedH, typename DerivedC>
    void fPropDropout(const MatrixBase<Derived> &data,
					  Engine &eng,
	   			   	  const MatrixBase<DerivedH> &h_t_minus_one,
	   			   	  const MatrixBase<DerivedC> &c_t_minus_one) { 
		fPropInputDropout(data, eng);
		fPropLSTMBlock(h_t_minus_one,
					   c_t_minus_one);
		h_t_dropout_layer.fProp(this->h_t,eng);

	}
	
	template<typename DerivedData>	
	void bPropInput(const MatrixBase<DerivedData> &data){
		input_node->bProp(data,
				o_t_node.bProp_matrix,
				i_t_node.bProp_matrix,
				f_t_node.bProp_matrix,
				tanh_c_prime_t_node.bProp_matrix);		
	}

	template<typename DerivedData>	
	void bPropInputDropout(const MatrixBase<DerivedData> &data){
		input_node->bPropDropout(data,
				o_t_node.bProp_matrix,
				i_t_node.bProp_matrix,
				f_t_node.bProp_matrix,
				tanh_c_prime_t_node.bProp_matrix);		
	}
			

	
	#ifdef PEEP

	template <typename DerivedH, typename DerivedC>
	void fPropLSTMBlock(const MatrixBase<DerivedH> &h_t_minus_one,
						const MatrixBase<DerivedC> &c_t_minus_one) {	
		//How much to remember
		W_h_to_i_node.param->fProp(h_t_minus_one,W_h_to_i_node.fProp_matrix);
		W_c_to_i_node.param->fProp(c_t_minus_one,W_c_to_i_node.fProp_matrix);
		i_t_input_matrix.noalias() = input_node->W_x_to_i_node.fProp_matrix + W_h_to_i_node.fProp_matrix + W_c_to_i_node.fProp_matrix;

		i_t_node.param->fProp(i_t_input_matrix,
							i_t_node.fProp_matrix);
							

		
		//How much to forget
		W_h_to_f_node.param->fProp(h_t_minus_one,W_h_to_f_node.fProp_matrix);
		W_c_to_f_node.param->fProp(c_t_minus_one,W_c_to_f_node.fProp_matrix);
		f_t_input_matrix.noalias() = input_node->W_x_to_f_node.fProp_matrix + W_h_to_f_node.fProp_matrix + W_c_to_f_node.fProp_matrix;
		f_t_node.param->fProp(f_t_input_matrix,
							f_t_node.fProp_matrix);

		//Generating the current cell value
		W_h_to_c_node.param->fProp(h_t_minus_one,W_h_to_c_node.fProp_matrix);	
		tanh_c_prime_t_input_matrix.noalias() = input_node->W_x_to_c_node.fProp_matrix + W_h_to_c_node.fProp_matrix;
		tanh_c_prime_t_node.param->fProp(tanh_c_prime_t_input_matrix,
										tanh_c_prime_t_node.fProp_matrix);
		

		c_t.array() = f_t_node.fProp_matrix.array()*c_t_minus_one.array() + 
				i_t_node.fProp_matrix.array()*tanh_c_prime_t_node.fProp_matrix.array();
		//cerr<<"c_t "<<c_t<<endl;
		//How much to scale the output

		W_h_to_o_node.param->fProp(h_t_minus_one,W_h_to_o_node.fProp_matrix);
		W_c_to_o_node.param->fProp(c_t,W_c_to_o_node.fProp_matrix);
		o_t_input_matrix.noalias() = input_node->W_x_to_o_node.fProp_matrix +  
						   W_h_to_o_node.fProp_matrix + 
						   W_c_to_o_node.fProp_matrix;

		o_t_node.param->fProp(o_t_input_matrix,
							o_t_node.fProp_matrix);	


		tanh_c_t_node.param->fProp(c_t,tanh_c_t_node.fProp_matrix);

		h_t.array() = o_t_node.fProp_matrix.array()*tanh_c_t_node.fProp_matrix.array();		

	}
	

		
	template<typename DerivedIn, typename DerivedDCIn, typename DerivedDHIn, typename DerivedH, typename DerivedC>
	void bPropLSTMBlock (const MatrixBase<DerivedIn> &d_Err_t_d_h_t,
			   const MatrixBase<DerivedDCIn> &d_Err_tPlusOne_to_n_d_c_t,
			   const MatrixBase<DerivedDHIn> &d_Err_tPlusOne_to_n_d_h_t,
			   const MatrixBase<DerivedH> &h_t_minus_one,
			   const MatrixBase<DerivedC> &c_t_minus_one, 
			   bool gradient_check,
			   bool norm_clipping,
			   int current_minibatch_size) {
		//Eigen::internal::set_is_malloc_allowed(false);			   
   		Matrix<precision_type,Dynamic,Dynamic> dummy_matrix;

   		//NOTE: d_Err_t_to_n_d_h_t is read as derivative of Error function from time t to n wrt h_t. 
   		//Similarly, d_Err_t_to_n_d_h_t is read as derivative of Error function from time t to n wrt c_t. 
   		//This is a slight abuse of notation. In our case, since we're maximizing log likelihood, we're taking derivatives of the negative of the 
   		//error function, which is the cross entropy.

		//Summing up the error derivatives for h_t from t+1,..,n and from t
   		d_Err_t_to_n_d_h_t = d_Err_t_d_h_t + d_Err_tPlusOne_to_n_d_h_t;
		
		//Derivative wrt o_t
   		d_Err_t_to_n_d_o_t.array() = d_Err_t_to_n_d_h_t.array()*tanh_c_t_node.fProp_matrix.array();

   		o_t_node.param->bProp(d_Err_t_to_n_d_o_t,
   						      o_t_node.bProp_matrix,
   							  dummy_matrix,
   							  o_t_node.fProp_matrix);// the third	 field does not matter. Its a dummy matrix


		//Summing up the error derivatives for h_t from t+1,..,n and from t					  
   		d_Err_t_to_n_d_tanh_c_t.array() = d_Err_t_to_n_d_h_t.array() * o_t_node.fProp_matrix.array();

   		tanh_c_t_node.param->bProp(d_Err_t_to_n_d_tanh_c_t,
   							tanh_c_t_node.bProp_matrix,
   							dummy_matrix,
   							tanh_c_t_node.fProp_matrix);

		
   		W_c_to_o_node.param->bProp(o_t_node.bProp_matrix,
   								W_c_to_o_node.bProp_matrix);

   		d_Err_t_to_n_d_c_t =  tanh_c_t_node.bProp_matrix + W_c_to_o_node.bProp_matrix + d_Err_tPlusOne_to_n_d_c_t;


   		//Error derivatives for f_t
   		d_Err_t_to_n_d_f_t.array() = d_Err_t_to_n_d_c_t.array()*c_t_minus_one.array();
   		//cerr<<"d_Err_t_to_n_d_f_t "<<d_Err_t_to_n_d_f_t<<endl;
   		f_t_node.param->bProp(d_Err_t_to_n_d_f_t,
   						      f_t_node.bProp_matrix,
   							  dummy_matrix,
   							  f_t_node.fProp_matrix);


   		//Error derivatives for i_t
   		d_Err_t_to_n_d_i_t.array() = d_Err_t_to_n_d_c_t.array()*tanh_c_prime_t_node.fProp_matrix.array();

   		i_t_node.param->bProp(d_Err_t_to_n_d_i_t,
   						      i_t_node.bProp_matrix,
   							  dummy_matrix,
   							  i_t_node.fProp_matrix);	

					  	
   		//Error derivatives for c_prime_t
   		d_Err_t_to_n_d_tanh_c_prime_t.array() = d_Err_t_to_n_d_c_t.array()*i_t_node.fProp_matrix.array();
 

   		tanh_c_prime_t_node.param->bProp(d_Err_t_to_n_d_tanh_c_prime_t,
   						      tanh_c_prime_t_node.bProp_matrix,
   							  dummy_matrix,
   							  tanh_c_prime_t_node.fProp_matrix);	
  								

   		//Error derivatives for h_t_minus_one
   		W_h_to_o_node.param->bProp(o_t_node.bProp_matrix,
   						 W_h_to_o_node.bProp_matrix);
    	W_h_to_f_node.param->bProp(f_t_node.bProp_matrix,
    						 W_h_to_f_node.bProp_matrix);
     	W_h_to_i_node.param->bProp(i_t_node.bProp_matrix,
     						 W_h_to_i_node.bProp_matrix);
 
      	W_h_to_c_node.param->bProp(tanh_c_prime_t_node.bProp_matrix,
      						 W_h_to_c_node.bProp_matrix);
   		d_Err_t_to_n_d_h_tMinusOne = W_h_to_o_node.bProp_matrix + 
   									W_h_to_f_node.bProp_matrix +
   									W_h_to_i_node.bProp_matrix +
   									W_h_to_c_node.bProp_matrix;		

   		//Error derivatives for c_t_minus_one
   		W_c_to_f_node.param->bProp(f_t_node.bProp_matrix,
   							W_c_to_f_node.bProp_matrix);
   		W_c_to_i_node.param->bProp(i_t_node.bProp_matrix,
   							W_c_to_i_node.bProp_matrix);	
							
   		d_Err_t_to_n_d_c_tMinusOne = (d_Err_t_to_n_d_c_t.array()*f_t_node.fProp_matrix.array()).matrix()+
   									W_c_to_f_node.bProp_matrix +
   									W_c_to_i_node.bProp_matrix;
		 

		
		

   		//Computing gradients of the paramters
   		//Derivative of weights out of h_t
   		//cerr<<"W_h_to_o_node"<<endl;
   	    W_h_to_o_node.param->updateGradient(o_t_node.bProp_matrix.leftCols(current_minibatch_size),
   											h_t_minus_one.leftCols(current_minibatch_size));
   	    //cerr<<"W_h_to_f_node"<<endl;										
   	    W_h_to_f_node.param->updateGradient(f_t_node.bProp_matrix.leftCols(current_minibatch_size),
   											h_t_minus_one.leftCols(current_minibatch_size));
   		//cerr<<"W_h_to_i_node"<<endl;									
   	    W_h_to_i_node.param->updateGradient(i_t_node.bProp_matrix.leftCols(current_minibatch_size),
   											h_t_minus_one.leftCols(current_minibatch_size));		
   		//cerr<<"W_h_to_c_node"<<endl;									
      		W_h_to_c_node.param->updateGradient(tanh_c_prime_t_node.bProp_matrix.leftCols(current_minibatch_size),
      						 					h_t_minus_one.leftCols(current_minibatch_size));
									
   		//Derivative of weights out of c_t and c_t_minus_one
   	    W_c_to_o_node.param->updateGradient(o_t_node.bProp_matrix.leftCols(current_minibatch_size),
   											this->c_t.leftCols(current_minibatch_size));
   	    W_c_to_i_node.param->updateGradient(i_t_node.bProp_matrix.leftCols(current_minibatch_size),
   											c_t_minus_one.leftCols(current_minibatch_size));
   	    W_c_to_f_node.param->updateGradient(f_t_node.bProp_matrix.leftCols(current_minibatch_size),
   											c_t_minus_one.leftCols(current_minibatch_size));		

   		// Updating the gradient of the hidden layer biases									
   		o_t_node.param->updateGradient(o_t_node.bProp_matrix.leftCols(current_minibatch_size));
   		f_t_node.param->updateGradient(f_t_node.bProp_matrix.leftCols(current_minibatch_size));
   		i_t_node.param->updateGradient(i_t_node.bProp_matrix.leftCols(current_minibatch_size));
   		tanh_c_prime_t_node.param->updateGradient(tanh_c_prime_t_node.bProp_matrix.leftCols(current_minibatch_size));	
		
		//updateGradient(h_t_minus_one,
		//			   c_t_minus_one,
		//			   current_minibatch_size);		
	}

	template <typename DerivedH, typename DerivedC> 
	void updateGradient(const MatrixBase<DerivedH> &h_t_minus_one,
						const MatrixBase<DerivedC> &c_t_minus_one,
						int current_minibatch_size){
   	    W_h_to_o_node.param->updateGradient(o_t_node.bProp_matrix.leftCols(current_minibatch_size),
   											h_t_minus_one.leftCols(current_minibatch_size));
   	    //cerr<<"W_h_to_f_node"<<endl;										
   	    W_h_to_f_node.param->updateGradient(f_t_node.bProp_matrix.leftCols(current_minibatch_size),
   											h_t_minus_one.leftCols(current_minibatch_size));
   		//cerr<<"W_h_to_i_node"<<endl;									
   	    W_h_to_i_node.param->updateGradient(i_t_node.bProp_matrix.leftCols(current_minibatch_size),
   											h_t_minus_one.leftCols(current_minibatch_size));		
   		//cerr<<"W_h_to_c_node"<<endl;									
      	W_h_to_c_node.param->updateGradient(tanh_c_prime_t_node.bProp_matrix.leftCols(current_minibatch_size),
      						 					h_t_minus_one.leftCols(current_minibatch_size));
									
   		//Derivative of weights out of c_t and c_t_minus_one
   	    W_c_to_o_node.param->updateGradient(o_t_node.bProp_matrix.leftCols(current_minibatch_size),
   											this->c_t.leftCols(current_minibatch_size));
   	    W_c_to_i_node.param->updateGradient(i_t_node.bProp_matrix.leftCols(current_minibatch_size),
   											c_t_minus_one.leftCols(current_minibatch_size));
   	    W_c_to_f_node.param->updateGradient(f_t_node.bProp_matrix.leftCols(current_minibatch_size),
   											c_t_minus_one.leftCols(current_minibatch_size));		

   		// Updating the gradient of the hidden layer biases									
   		o_t_node.param->updateGradient(o_t_node.bProp_matrix.leftCols(current_minibatch_size));
   		f_t_node.param->updateGradient(f_t_node.bProp_matrix.leftCols(current_minibatch_size));
   		i_t_node.param->updateGradient(i_t_node.bProp_matrix.leftCols(current_minibatch_size));
   		tanh_c_prime_t_node.param->updateGradient(tanh_c_prime_t_node.bProp_matrix.leftCols(current_minibatch_size));			
	}
	#else

	//fProp without peeps

	template <typename DerivedH, typename DerivedC>
	void fPropLSTMBlock(const MatrixBase<DerivedH> &h_t_minus_one,
						const MatrixBase<DerivedC> &c_t_minus_one) {
		//std::cerr<<"x to i fprop"<<W_x_to_i_node.fProp_matrix<<std::endl;
		W_h_to_i_node.param->fProp(h_t_minus_one,W_h_to_i_node.fProp_matrix);
		//W_x_to_c_node.param->fProp(input_layer_node.fProp_matrix,W_x_to_c_node.fProp_matrix);
		W_h_to_c_node.param->fProp(h_t_minus_one,W_h_to_c_node.fProp_matrix);	
		W_h_to_o_node.param->fProp(h_t_minus_one,W_h_to_o_node.fProp_matrix);
		W_h_to_f_node.param->fProp(h_t_minus_one,W_h_to_f_node.fProp_matrix);
			

		i_t_input_matrix.noalias() = input_node->W_x_to_i_node.fProp_matrix + W_h_to_i_node.fProp_matrix;
		f_t_input_matrix.noalias() = input_node->W_x_to_f_node.fProp_matrix + W_h_to_f_node.fProp_matrix;
	
		//Computing input and forget gates
		//cerr<<"i t input matrix"<<i_t_input_matrix<<endl;
		i_t_node.param->fProp(i_t_input_matrix,
							i_t_node.fProp_matrix);
		//cerr<<"i_t_node.fProp_matrix "<<endl<<i_t_node.fProp_matrix<<endl;
		//How much to forget					
		f_t_node.param->fProp(f_t_input_matrix,
							f_t_node.fProp_matrix);		


		//computing c_prime_t


		tanh_c_prime_t_input_matrix.noalias() = input_node->W_x_to_c_node.fProp_matrix + W_h_to_c_node.fProp_matrix;
		tanh_c_prime_t_node.param->fProp(tanh_c_prime_t_input_matrix,
										tanh_c_prime_t_node.fProp_matrix);
	

	
		//Computing the current cell value

		c_t.array() = f_t_node.fProp_matrix.array()*c_t_minus_one.array() + 
				i_t_node.fProp_matrix.array()*tanh_c_prime_t_node.fProp_matrix.array();


		//How much to scale the output

		o_t_input_matrix.noalias() = input_node->W_x_to_o_node.fProp_matrix +  
						   W_h_to_o_node.fProp_matrix;
					   

		//Computing the output gate
		//std::cerr<<"o t input matrix is "<<o_t_input_matrix<<std::endl;
		o_t_node.param->fProp(o_t_input_matrix,
							o_t_node.fProp_matrix);	

		//computing the hidden layer
		tanh_c_t_node.param->fProp(c_t,tanh_c_t_node.fProp_matrix);
		//<<"tanh_c_t_node.fProp_matrix is "<<tanh_c_t_node.fProp_matrix<<endl;
		h_t.array() = o_t_node.fProp_matrix.array()*tanh_c_t_node.fProp_matrix.array();		

	}


	
	template<typename DerivedIn, typename DerivedDCIn, typename DerivedDHIn, typename DerivedH, typename DerivedC>
	void bPropLSTMBlock (const MatrixBase<DerivedIn> &d_Err_t_d_h_t,
			   const MatrixBase<DerivedDCIn> &d_Err_tPlusOne_to_n_d_c_t,
			   const MatrixBase<DerivedDHIn> &d_Err_tPlusOne_to_n_d_h_t,
			   const MatrixBase<DerivedH> &h_t_minus_one,
			   const MatrixBase<DerivedC> &c_t_minus_one,			   
			   bool gradient_check,
			   bool norm_clipping,
			   int current_minibatch_size) {	   
		Matrix<precision_type,Dynamic,Dynamic> dummy_matrix;

		//NOTE: d_Err_t_to_n_d_h_t is read as derivative of Error function from time t to n wrt h_t. 
		//Similarly, d_Err_t_to_n_d_h_t is read as derivative of Error function from time t to n wrt c_t. 
		//This is a slight abuse of notation. In our case, since we're maximizing log likelihood, we're taking derivatives of the negative of the 
		//error function, which is the cross entropy.

		//Error derivatives for h_t

		d_Err_t_to_n_d_h_t = d_Err_t_d_h_t + d_Err_tPlusOne_to_n_d_h_t;

		//Error derivativs for o_t
		d_Err_t_to_n_d_o_t.array() = d_Err_t_to_n_d_h_t.array()*tanh_c_t_node.fProp_matrix.array();

		//cerr<<"O t node fProp matrix is "<<o_t_node.fProp_matrix<<endl;
		o_t_node.param->bProp(d_Err_t_to_n_d_o_t,
						      o_t_node.bProp_matrix,
							  dummy_matrix,
							  o_t_node.fProp_matrix);// the third	 field does not matter. Its a dummy matrix


		//Error derivatives for tanh_c_t					   
		//d_Err_t_to_n_d_tanh_c_t.array() = d_Err_t_d_h_t.array() * o_t_node.fProp_matrix.array();// THIS WAS THE WRONG GRADIENT!!
		d_Err_t_to_n_d_tanh_c_t.array() = d_Err_t_to_n_d_h_t.array() * o_t_node.fProp_matrix.array();
		//cerr<<"d_Err_t_to_n_d_tanh_c_t "<<d_Err_t_to_n_d_tanh_c_t<<endl;
		tanh_c_t_node.param->bProp(d_Err_t_to_n_d_tanh_c_t,
							tanh_c_t_node.bProp_matrix,
							dummy_matrix,
							tanh_c_t_node.fProp_matrix);


		//Error derivatives for c_t
		d_Err_t_to_n_d_c_t.noalias() =  tanh_c_t_node.bProp_matrix + d_Err_tPlusOne_to_n_d_c_t;

		//Error derivatives for f_t
		//cerr<<"c_t_minus_one size "<<c_t_minus_one.rows()<<" "<<c_t_minus_one.cols()<<endl;
		//cerr<<"c_t_minus_one "<<c_t_minus_one<<endl;
		d_Err_t_to_n_d_f_t.array() = d_Err_t_to_n_d_c_t.array()*c_t_minus_one.array();
		//cerr<<"d_Err_t_to_n_d_f_t "<<d_Err_t_to_n_d_f_t<<endl;
		f_t_node.param->bProp(d_Err_t_to_n_d_f_t,
						      f_t_node.bProp_matrix,
							  dummy_matrix,
							  f_t_node.fProp_matrix);

		//Error derivatives for i_t
		d_Err_t_to_n_d_i_t.array() = d_Err_t_to_n_d_c_t.array()*tanh_c_prime_t_node.fProp_matrix.array();
		//cerr<<"d_Err_t_to_n_d_i_t "<<d_Err_t_to_n_d_i_t<<endl;
		i_t_node.param->bProp(d_Err_t_to_n_d_i_t,
						      i_t_node.bProp_matrix,
							  dummy_matrix,
							  i_t_node.fProp_matrix);	

		//Error derivatives for c_prime_t
		d_Err_t_to_n_d_tanh_c_prime_t.array() = d_Err_t_to_n_d_c_t.array()*i_t_node.fProp_matrix.array();

		tanh_c_prime_t_node.param->bProp(d_Err_t_to_n_d_tanh_c_prime_t,
						      tanh_c_prime_t_node.bProp_matrix,
							  dummy_matrix,
							  tanh_c_prime_t_node.fProp_matrix);	

		//Error derivatives for h_t_minus_one
			W_h_to_o_node.param->bProp(o_t_node.bProp_matrix,
						 W_h_to_o_node.bProp_matrix);
			W_h_to_f_node.param->bProp(f_t_node.bProp_matrix,
							 W_h_to_f_node.bProp_matrix);
	 		W_h_to_i_node.param->bProp(i_t_node.bProp_matrix,
	 						 W_h_to_i_node.bProp_matrix);
		//cerr<<"tanh_c_prime_t_node.bProp_matrix "<<tanh_c_prime_t_node.bProp_matrix<<endl;
	  		W_h_to_c_node.param->bProp(tanh_c_prime_t_node.bProp_matrix,
	  						 W_h_to_c_node.bProp_matrix);
		d_Err_t_to_n_d_h_tMinusOne = W_h_to_o_node.bProp_matrix + 
									W_h_to_f_node.bProp_matrix +
									W_h_to_i_node.bProp_matrix +
									W_h_to_c_node.bProp_matrix;		

			
		d_Err_t_to_n_d_c_tMinusOne.noalias() = (d_Err_t_to_n_d_c_t.array()*f_t_node.fProp_matrix.array()).matrix();
	

		/*
		//Computing gradients of the paramters
		//Derivative of weights out of h_t
		//cerr<<"W_h_to_o_node"<<endl;
	    W_h_to_o_node.param->updateGradient(o_t_node.bProp_matrix.leftCols(current_minibatch_size),
											h_t_minus_one.leftCols(current_minibatch_size));
	    //cerr<<"W_h_to_f_node"<<endl;										
	    W_h_to_f_node.param->updateGradient(f_t_node.bProp_matrix.leftCols(current_minibatch_size),
											h_t_minus_one.leftCols(current_minibatch_size));
		//cerr<<"W_h_to_i_node"<<endl;									
	    W_h_to_i_node.param->updateGradient(i_t_node.bProp_matrix.leftCols(current_minibatch_size),
											h_t_minus_one.leftCols(current_minibatch_size));		
		//cerr<<"W_h_to_c_node"<<endl;									
	  	W_h_to_c_node.param->updateGradient(tanh_c_prime_t_node.bProp_matrix.leftCols(current_minibatch_size),
	  						 					h_t_minus_one.leftCols(current_minibatch_size));
	
								


		// Updating the gradient of the hidden layer biases									
		o_t_node.param->updateGradient(o_t_node.bProp_matrix.leftCols(current_minibatch_size));
		f_t_node.param->updateGradient(f_t_node.bProp_matrix.leftCols(current_minibatch_size));
		i_t_node.param->updateGradient(i_t_node.bProp_matrix.leftCols(current_minibatch_size));
		tanh_c_prime_t_node.param->updateGradient(tanh_c_prime_t_node.bProp_matrix.leftCols(current_minibatch_size));	
		*/
		updateGradient(h_t_minus_one,
					   c_t_minus_one,
					   current_minibatch_size);	   	
	}	
	
	template <typename DerivedH, typename DerivedC> 
	void updateGradient(const MatrixBase<DerivedH> &h_t_minus_one,
						const MatrixBase<DerivedC> &c_t_minus_one,
						int current_minibatch_size){
		//Computing gradients of the paramters
		//Derivative of weights out of h_t
		//cerr<<"W_h_to_o_node"<<endl;
	    W_h_to_o_node.param->updateGradient(o_t_node.bProp_matrix.leftCols(current_minibatch_size),
											h_t_minus_one.leftCols(current_minibatch_size));
	    //cerr<<"W_h_to_f_node"<<endl;										
	    W_h_to_f_node.param->updateGradient(f_t_node.bProp_matrix.leftCols(current_minibatch_size),
											h_t_minus_one.leftCols(current_minibatch_size));
		//cerr<<"W_h_to_i_node"<<endl;									
	    W_h_to_i_node.param->updateGradient(i_t_node.bProp_matrix.leftCols(current_minibatch_size),
											h_t_minus_one.leftCols(current_minibatch_size));		
		//cerr<<"W_h_to_c_node"<<endl;									
	  	W_h_to_c_node.param->updateGradient(tanh_c_prime_t_node.bProp_matrix.leftCols(current_minibatch_size),
	  						 					h_t_minus_one.leftCols(current_minibatch_size));

			


		// Updating the gradient of the hidden layer biases									
		o_t_node.param->updateGradient(o_t_node.bProp_matrix.leftCols(current_minibatch_size));
		f_t_node.param->updateGradient(f_t_node.bProp_matrix.leftCols(current_minibatch_size));
		i_t_node.param->updateGradient(i_t_node.bProp_matrix.leftCols(current_minibatch_size));
		tanh_c_prime_t_node.param->updateGradient(tanh_c_prime_t_node.bProp_matrix.leftCols(current_minibatch_size));								
	}	
	
	#endif

	
	//This takes the sequence continuation indices, the previous hidden and cell states and creates new ones for this LSTM block
	template <typename DerivedH, typename DerivedC>//, typename DerivedS>
	void copyToHiddenStates(const MatrixBase<DerivedH> &h_t_minus_one,
							const MatrixBase<DerivedC> &c_t_minus_one) {

						int current_minibatch_size = h_t_minus_one.cols();	
						#pragma omp parallel for 
						for (int index=0; index<current_minibatch_size; index++){ 
							//UNCONST(DerivedS,const_sequence_cont_indices,sequence_cont_indices);		
				
							//cerr<<"current minibatch size "<<current_minibatch_size<<endl;
							if (0) { // sequence_cont_indices(index) == 0) {
								this->h_t_minus_one.col(index).setZero(); 			
								this->c_t_minus_one.col(index).setZero();
							} else {
								//cerr<<"copying"<<endl;
								this->h_t_minus_one.col(index) = h_t_minus_one.col(index);
								this->c_t_minus_one.col(index) = c_t_minus_one.col(index);
								//this->c_t_minus_one.col(index) = c_t_minus_one.col(index).array().unaryExpr(stateClipper());
							}
						}	
												

		
	}

	//This takes the sequence continuation indices, the previous hidden and cell states and creates new ones for this LSTM block
	template <typename DerivedH, typename DerivedC , typename DerivedS>
	static void filterStatesAndErrors(const MatrixBase<DerivedH> &from_h_matrix,
							const MatrixBase<DerivedC> &from_c_matrix,
							const MatrixBase<DerivedH> &const_to_h_matrix,
							const MatrixBase<DerivedC> &const_to_c_matrix,
							const Eigen::ArrayBase<DerivedS> &sequence_cont_indices) {
						int current_minibatch_size = sequence_cont_indices.cols();	
						UNCONST(DerivedC, const_to_c_matrix, to_c_matrix);
						UNCONST(DerivedH, const_to_h_matrix, to_h_matrix);

						#pragma omp parallel for 
						for (int index=0; index<current_minibatch_size; index++){ 
							//UNCONST(DerivedS,const_sequence_cont_indices,sequence_cont_indices);		
							//cerr<<"current minibatch size "<<current_minibatch_size<<endl;
							if (sequence_cont_indices(index) == 0) {
								to_h_matrix.col(index).setZero(); 			
								to_c_matrix.col(index).setZero();

							} else {

								to_h_matrix.col(index) = from_h_matrix.col(index);
								to_c_matrix.col(index) = from_c_matrix.col(index);

							}
						}	

		
	}

	//This takes the sequence continuation indices, the previous hidden and cell states and creates new ones for this LSTM block/
	//It also takes a list that says where the new hidden states must be obtained from. This is very useful for beam search.
	//In beam search, the hidden states must come from the current k-best sequence. The current k-best sequence might be continuations
	//of the same item or different items in the previous k-best sequence and thus we can't just copy over the previous hidden states
	//because some of the k-best sequences corresponding to previous hidden states might have died. 
	//TODO: should the indices be a vector ? 
	template <typename DerivedH, typename DerivedC , typename indexType>
	static void copyKBestHiddenStates(const MatrixBase<DerivedH> &from_h_matrix,
							const MatrixBase<DerivedC> &from_c_matrix,
							const MatrixBase<DerivedH> &const_to_h_matrix,
							const MatrixBase<DerivedC> &const_to_c_matrix,
							const vector<indexType> &from_indices) {
						int current_minibatch_size = from_indices.size();
				
						UNCONST(DerivedC, const_to_c_matrix, to_c_matrix);
						UNCONST(DerivedH, const_to_h_matrix, to_h_matrix);
		
										
						//int current_minibatch_size = h_t_minus_one.cols();	
						//#pragma omp parallel for 
						for (int index=0; index<current_minibatch_size; index++){ 
							//UNCONST(DerivedS,const_sequence_cont_indices,sequence_cont_indices);		
							//cerr<<"current minibatch size "<<current_minibatch_size<<endl;
								//cerr<<"from_indices.at(index) "<<from_indices.at(index)<<endl;
								to_h_matrix.col(index) = from_h_matrix.col(from_indices.at(index)); 			
								to_c_matrix.col(index) = from_c_matrix.col(from_indices.at(index));

						}	

	}
			
	//For stability, the gradient of the inputs of the loss to the LSTM is clipped, that is before applying the tanh and sigmoid
	//nonlinearities 
	void clipGradient(){}
	
	void resetGradient(){
		
	}	
    //Question, should the state gather be a part of 
    template<typename DerivedO, 
				typename DerivedF, 
				typename DerivedI, 
				typename DerivedH, 
				typename DerivedC>
    void getInternals(const MatrixBase<DerivedH> &const_get_h_t,
					const MatrixBase<DerivedC>   &const_get_c_t,
					const MatrixBase<DerivedF>   &const_get_f_t,
					const MatrixBase<DerivedI>   &const_get_i_t,
					const MatrixBase<DerivedO>   &const_get_o_t,
					const int sent_index){
		UNCONST(DerivedH, const_get_h_t, get_h_t);
		UNCONST(DerivedH, const_get_c_t, get_c_t);
		UNCONST(DerivedH, const_get_f_t, get_f_t);
		UNCONST(DerivedH, const_get_i_t, get_i_t);
		UNCONST(DerivedH, const_get_o_t, get_o_t);
		get_h_t = h_t.col(sent_index);
		//cerr<<"graph classes h_t.col("<<sent_index<<") "<<h_t.col(sent_index)<<endl;
		//cerr<<"graph classes c_t.col("<<sent_index<<") "<<c_t.col(sent_index)<<endl;
		get_c_t = c_t.col(sent_index);
		get_f_t = f_t_node.fProp_matrix.col(sent_index);
		get_i_t = i_t_node.fProp_matrix.col(sent_index);	
		get_o_t = o_t_node.fProp_matrix.col(sent_index);
    }

	
};

//Node that takes takes in the two
class Bidirectional_combiner_node{
	int minibatch_size;
public:
	Node<Linear_layer> forward_layer_transformation_node, backward_layer_transformation_node;
	Node<Activation_function> combination_layer_node;
	Matrix<precision_type, Dynamic, Dynamic> combination_layer_input_matrix, dummy_matrix;
	
	Bidirectional_combiner_node():
		minibatch_size(0),
		forward_layer_transformation_node(),
		backward_layer_transformation_node(),
		combination_layer_node(),
		combination_layer_input_matrix(Matrix<precision_type, Dynamic, Dynamic>()),
		dummy_matrix(Matrix<precision_type, Dynamic, Dynamic>()) {}
	Bidirectional_combiner_node(bidirectional_combiner &combiner,
								int minibatch_size):
		minibatch_size(minibatch_size),
		forward_layer_transformation_node(&combiner.forward_layer_transformation_layer, minibatch_size),
		backward_layer_transformation_node(&combiner.backward_layer_transformation_layer, minibatch_size),
		combination_layer_node(&combiner.combination_layer, minibatch_size) { }
		
	void resize(int minibatch_size){
		this->minibatch_size = minibatch_size;
		forward_layer_transformation_node.resize(minibatch_size);
		backward_layer_transformation_node.resize(minibatch_size);
		combination_layer_node.resize(minibatch_size);
		combination_layer_input_matrix.setZero(
			combination_layer_node.param->n_inputs(),
			forward_layer_transformation_node.param->n_outputs());
	}

	template <typename Derived>
	void fProp(const MatrixBase<Derived> &forward_layer_fProp_matrix,
				const MatrixBase<Derived> &backward_layer_fProp_matrix){
		//cerr<<"Data is "<<data<<endl;
		forward_layer_transformation_node.param->fProp(forward_layer_fProp_matrix,
										forward_layer_transformation_node.fProp_matrix);
		backward_layer_transformation_node.param->fProp(backward_layer_fProp_matrix,
										backward_layer_transformation_node.fProp_matrix);
		combination_layer_input_matrix.noalias() =
			forward_layer_transformation_node.fProp_matrix+
			backward_layer_transformation_node.fProp_matrix;
		combination_layer_node.param->fProp(combination_layer_input_matrix,
			combination_layer_node.fProp_matrix);
	}		
	
	template <typename DerivedGIn, typename DerivedIn>
	void bProp(const MatrixBase<DerivedGIn> &bProp_matrix,
			   const MatrixBase<DerivedIn> &forward_layer_fProp_matrix,
			   const MatrixBase<DerivedIn> &backward_layer_fProp_matrix){
		int current_minibatch_size = bProp_matrix.cols();
		combination_layer_node.param->bProp(bProp_matrix,
   						      combination_layer_node.bProp_matrix.leftCols(current_minibatch_size),
   							  combination_layer_node.fProp_matrix.leftCols(current_minibatch_size),
   							  dummy_matrix);
		forward_layer_transformation_node.param->bProp(
			combination_layer_node.bProp_matrix,
			forward_layer_transformation_node.bProp_matrix);
		backward_layer_transformation_node.param->bProp(
			combination_layer_node.bProp_matrix,
			backward_layer_transformation_node.bProp_matrix);	
		//Updating the gradients
		forward_layer_transformation_node.param->updateGradient(
			combination_layer_node.bProp_matrix.leftCols(current_minibatch_size),
			forward_layer_fProp_matrix.leftCols(current_minibatch_size));
		backward_layer_transformation_node.param->updateGradient(
			combination_layer_node.bProp_matrix.leftCols(current_minibatch_size),
			backward_layer_fProp_matrix.leftCols(current_minibatch_size));
			
	}
};

class Standard_input_node{
	int minibatch_size;
public:
	//Each LSTM node has a bunch of nodes and temporary data structures
    Node<Input_word_embeddings> W_x_to_i_node, W_x_to_f_node, W_x_to_c_node, W_x_to_o_node;
	
	Standard_input_node():
		minibatch_size(0),
		W_x_to_i_node(),
		W_x_to_f_node(),
		W_x_to_c_node(),
		W_x_to_o_node() {}	
		
	Standard_input_node(standard_input_model &input, int minibatch_size): 
		W_x_to_i_node(&input.W_x_to_i, minibatch_size),
		W_x_to_f_node(&input.W_x_to_f, minibatch_size),
		W_x_to_c_node(&input.W_x_to_c, minibatch_size),
		W_x_to_o_node(&input.W_x_to_o, minibatch_size),
		minibatch_size(minibatch_size) {
			//cerr<<"The input embeddings are"<<*(W_x_to_i_node.param->get_W())<<endl;
		}

	//Resizing all the parameters
	void resize(int minibatch_size){
		//cerr<<"Resizing the input node"<<endl;
		this->minibatch_size = minibatch_size;
		W_x_to_i_node.resize(minibatch_size);
		W_x_to_f_node.resize(minibatch_size);
		W_x_to_c_node.resize(minibatch_size);
		W_x_to_o_node.resize(minibatch_size);
	}
	template<typename Derived, typename DropMask>
    void fPropInputDropout(const MatrixBase<Derived> &data,
					  const MatrixBase<DropMask> &dropout_mask) {
					  	
	}	
					
	template <typename Derived>
	void fProp(const MatrixBase<Derived> &data){
		//cerr<<"Data is "<<data<<endl;
		W_x_to_c_node.param->fProp(data,W_x_to_c_node.fProp_matrix);
		W_x_to_f_node.param->fProp(data,W_x_to_f_node.fProp_matrix);
		W_x_to_o_node.param->fProp(data,W_x_to_o_node.fProp_matrix);
		W_x_to_i_node.param->fProp(data,W_x_to_i_node.fProp_matrix);			
	}	
	
	template<typename DerivedData, typename DerivedDIn>
	void bProp(const MatrixBase<DerivedData> &data,
				const MatrixBase<DerivedDIn> &o_t_node_bProp_matrix,
				const MatrixBase<DerivedDIn> &i_t_node_bProp_matrix,
				const MatrixBase<DerivedDIn> &f_t_node_bProp_matrix,
				const MatrixBase<DerivedDIn> &tanh_c_prime_t_node_bProp_matrix){
		//cerr<<"input_layer_node.fProp_matrix is "<<input_layer_node.fProp_matrix<<endl;
		//cerr<<"W_x_to_o_node"<<endl;
		int current_minibatch_size = data.cols();
		W_x_to_o_node.param->updateGradient(o_t_node_bProp_matrix.leftCols(current_minibatch_size),
											data);
		//cerr<<"W_x_to_i_node"<<endl;									
		W_x_to_i_node.param->updateGradient(i_t_node_bProp_matrix.leftCols(current_minibatch_size),
											data);
		//cerr<<"W_x_to_f_node"<<endl;									
		W_x_to_f_node.param->updateGradient(f_t_node_bProp_matrix.leftCols(current_minibatch_size),
											data);	
		//cerr<<"W_x_to_c_node"<<endl;									
		W_x_to_c_node.param->updateGradient(tanh_c_prime_t_node_bProp_matrix.leftCols(current_minibatch_size),
											data);			
																	
	}
};

class Google_input_node{
	int minibatch_size;
public:
	//Each LSTM node has a bunch of nodes and temporary data structures
    Node<Input_word_embeddings> input_layer_node;
    Node<Linear_layer> W_x_to_i_node, W_x_to_f_node, W_x_to_c_node, W_x_to_o_node;
	Eigen::Matrix<precision_type,Eigen::Dynamic,Eigen::Dynamic> d_Err_t_to_n_d_x_t;
	//Eigen::Matrix<precision_type,Eigen::Dynamic,Eigen::Dynamic> dropout_mask;
	Dropout_layer x_t_dropout_layer;
	Google_input_node():
		minibatch_size(0),
		input_layer_node(),
		W_x_to_i_node(),
		W_x_to_f_node(),
		W_x_to_c_node(),
		W_x_to_o_node() ,
		d_Err_t_to_n_d_x_t(Eigen::Matrix<precision_type,Eigen::Dynamic,Eigen::Dynamic>()),
		x_t_dropout_layer(Dropout_layer()){}	
		
	Google_input_node(google_input_model &input, int minibatch_size): 
		input_layer_node(input.input_layer, minibatch_size),
		W_x_to_i_node(&input.W_x_to_i, minibatch_size),
		W_x_to_f_node(&input.W_x_to_f, minibatch_size),
		W_x_to_c_node(&input.W_x_to_c, minibatch_size),
		W_x_to_o_node(&input.W_x_to_o, minibatch_size),
		x_t_dropout_layer(Dropout_layer()),
		minibatch_size(minibatch_size) {
			//cerr<<"The input embeddings are"<<*(W_x_to_i_node.param->get_W())<<endl;
		}

	//Resizing all the parameters
	void resize(int minibatch_size,
				double dropout_probability){
		//cerr<<"Resizing the input node"<<endl;
		this->minibatch_size = minibatch_size;
		input_layer_node.resize(minibatch_size);
		W_x_to_i_node.resize(minibatch_size);
		W_x_to_f_node.resize(minibatch_size);
		W_x_to_c_node.resize(minibatch_size);
		W_x_to_o_node.resize(minibatch_size);
		d_Err_t_to_n_d_x_t.resize(input_layer_node.param->n_outputs(),minibatch_size);

	}
	
	void resizeDropout(int minibatch_size,
						precision_type dropout_probability){
		resize (minibatch_size);
		//If using dropout
		//cerr<<"In resize dropout "<<endl;
		//getchar();
		if (dropout_probability > 0) {
			x_t_dropout_layer = Dropout_layer(input_layer_node.param->n_outputs(), 
											minibatch_size, 
											1.-dropout_probability);
		} else {
			cerr<<"The dropout probability has to be greater than 0!"<<endl;
			exit(1);
		}			
	}

	//Resizing all the parameters
	void resize(int minibatch_size){
		//cerr<<"Resizing the input node"<<endl;
		this->minibatch_size = minibatch_size;
		input_layer_node.resize(minibatch_size);
		W_x_to_i_node.resize(minibatch_size);
		W_x_to_f_node.resize(minibatch_size);
		W_x_to_c_node.resize(minibatch_size);
		W_x_to_o_node.resize(minibatch_size);
		d_Err_t_to_n_d_x_t.resize(input_layer_node.param->n_outputs(),minibatch_size);
	}
	
	
	template <typename Derived>
	void fProp(const MatrixBase<Derived> &data){
		fPropInput(data);
		fPropProjections();
	}
	
	template <typename Derived, typename Engine>
	void fPropDropout(const MatrixBase<Derived> &data,
						Engine &eng){
		//cerr<<"in fProp dropout"<<endl;
		fPropInput(data);
		//cerr<<"input_layer_node.fProp_matrix "<<input_layer_node.fProp_matrix<<endl;
		x_t_dropout_layer.fProp(input_layer_node.fProp_matrix,eng);
		//cerr<<"x_t_dropout_layer.fProp_matrix "<<input_layer_node.fProp_matrix<<endl;
		//getchar();
		fPropProjections();
	}
	
	
	template <typename Derived>
	void fPropInput(const MatrixBase<Derived> &data){
		input_layer_node.param->fProp(data, input_layer_node.fProp_matrix);
		//cerr<<"input_layer_node.fProp_matrix"<<endl<<input_layer_node.fProp_matrix<<endl;
	}
	
	
	void fPropProjections(){
		W_x_to_c_node.param->fProp(input_layer_node.fProp_matrix,W_x_to_c_node.fProp_matrix);
		//cerr<<"W_x_to_c_node.fProp_matrix "<<endl<<W_x_to_c_node.fProp_matrix<<endl;
		W_x_to_f_node.param->fProp(input_layer_node.fProp_matrix,W_x_to_f_node.fProp_matrix);
		//cerr<<"W_x_to_f_node.fProp_matrix "<<endl<<W_x_to_f_node.fProp_matrix<<endl;
		W_x_to_o_node.param->fProp(input_layer_node.fProp_matrix,W_x_to_o_node.fProp_matrix);
		//cerr<<"W_x_to_o_node.fProp_matrix "<<endl<<W_x_to_o_node.fProp_matrix<<endl;
		W_x_to_i_node.param->fProp(input_layer_node.fProp_matrix,W_x_to_i_node.fProp_matrix);		
	}
	

	
	template<typename DerivedData, typename DerivedDIn>
	void bProp(const MatrixBase<DerivedData> &data,
				const MatrixBase<DerivedDIn> &o_t_node_bProp_matrix,
				const MatrixBase<DerivedDIn> &i_t_node_bProp_matrix,
				const MatrixBase<DerivedDIn> &f_t_node_bProp_matrix,
				const MatrixBase<DerivedDIn> &tanh_c_prime_t_node_bProp_matrix){
		int current_minibatch_size = data.cols();
		bPropProjections(o_t_node_bProp_matrix,
				i_t_node_bProp_matrix,
				f_t_node_bProp_matrix,
				tanh_c_prime_t_node_bProp_matrix,
				current_minibatch_size);
		bPropInput(data);
					
	}
	template<typename DerivedData, typename DerivedDIn>
	void bPropDropout(const MatrixBase<DerivedData> &data,
				const MatrixBase<DerivedDIn> &o_t_node_bProp_matrix,
				const MatrixBase<DerivedDIn> &i_t_node_bProp_matrix,
				const MatrixBase<DerivedDIn> &f_t_node_bProp_matrix,
				const MatrixBase<DerivedDIn> &tanh_c_prime_t_node_bProp_matrix) {
		int current_minibatch_size = data.cols();			
		bPropProjections(o_t_node_bProp_matrix,
				i_t_node_bProp_matrix,
				f_t_node_bProp_matrix,
				tanh_c_prime_t_node_bProp_matrix,
				current_minibatch_size);
		x_t_dropout_layer.bProp(d_Err_t_to_n_d_x_t);
		bPropInput(data);				
	}
					
						
	template <typename DerivedDIn>
	void bPropProjections(const MatrixBase<DerivedDIn> &o_t_node_bProp_matrix,
				const MatrixBase<DerivedDIn> &i_t_node_bProp_matrix,
				const MatrixBase<DerivedDIn> &f_t_node_bProp_matrix,
				const MatrixBase<DerivedDIn> &tanh_c_prime_t_node_bProp_matrix,
				int current_minibatch_size){
				//int current_minibatch_size = data.cols();
				W_x_to_c_node.param->bProp(tanh_c_prime_t_node_bProp_matrix,
										W_x_to_c_node.bProp_matrix);
				W_x_to_o_node.param->bProp(o_t_node_bProp_matrix,
										W_x_to_o_node.bProp_matrix);
				W_x_to_f_node.param->bProp(f_t_node_bProp_matrix,
										W_x_to_f_node.bProp_matrix);
				W_x_to_i_node.param->bProp(i_t_node_bProp_matrix,
										W_x_to_i_node.bProp_matrix);

			
				W_x_to_o_node.param->updateGradient(o_t_node_bProp_matrix.leftCols(current_minibatch_size),
													input_layer_node.fProp_matrix.leftCols(current_minibatch_size));
				//cerr<<"W_x_to_i_node"<<endl;									
				W_x_to_i_node.param->updateGradient(i_t_node_bProp_matrix.leftCols(current_minibatch_size),
													input_layer_node.fProp_matrix.leftCols(current_minibatch_size));
				//cerr<<"W_x_to_f_node"<<endl;									
				W_x_to_f_node.param->updateGradient(f_t_node_bProp_matrix.leftCols(current_minibatch_size),
													input_layer_node.fProp_matrix.leftCols(current_minibatch_size));	
				//cerr<<"W_x_to_c_node"<<endl;									
				W_x_to_c_node.param->updateGradient(tanh_c_prime_t_node_bProp_matrix.leftCols(current_minibatch_size),
													input_layer_node.fProp_matrix.leftCols(current_minibatch_size));		
				d_Err_t_to_n_d_x_t = W_x_to_c_node.bProp_matrix + 
									W_x_to_o_node.bProp_matrix +
									W_x_to_f_node.bProp_matrix +
									W_x_to_i_node.bProp_matrix;												
				
	}
	
	template<typename DerivedData>
	void bPropInput(const MatrixBase<DerivedData> &data) {
		int current_minibatch_size = data.cols();
		input_layer_node.param->updateGradient(d_Err_t_to_n_d_x_t.leftCols(current_minibatch_size),
									data);				
	}

};

class Hidden_to_hidden_input_node{
	int minibatch_size;
public:
	//Each LSTM node has a bunch of nodes and temporary data structures
    Node<Linear_layer> W_x_to_i_node, W_x_to_f_node, W_x_to_c_node, W_x_to_o_node;
	Eigen::Matrix<precision_type,Eigen::Dynamic,Eigen::Dynamic> d_Err_t_to_n_d_x_t;
		
	Hidden_to_hidden_input_node():
		minibatch_size(0),
		W_x_to_i_node(),
		W_x_to_f_node(),
		W_x_to_c_node(),
		W_x_to_o_node() {}	
		
	Hidden_to_hidden_input_node(hidden_to_hidden_input_model &input, int minibatch_size): 
		W_x_to_i_node(&input.W_x_to_i, minibatch_size),
		W_x_to_f_node(&input.W_x_to_f, minibatch_size),
		W_x_to_c_node(&input.W_x_to_c, minibatch_size),
		W_x_to_o_node(&input.W_x_to_o, minibatch_size),
		minibatch_size(minibatch_size) {
			//cerr<<"The input embeddings are"<<*(W_x_to_i_node.param->get_W())<<endl;
		}

	//Resizing all the parameters
	void resize(int minibatch_size){
		//cerr<<"Resizing the input node"<<endl;
		this->minibatch_size = minibatch_size;
		W_x_to_i_node.resize(minibatch_size);
		W_x_to_f_node.resize(minibatch_size);
		W_x_to_c_node.resize(minibatch_size);
		W_x_to_o_node.resize(minibatch_size);
		d_Err_t_to_n_d_x_t.resize(W_x_to_o_node.param->n_inputs(),minibatch_size);
	}
	
	template<typename Derived, typename DropMask>
    void fPropInputDropout(const MatrixBase<Derived> &data,
					  const MatrixBase<DropMask> &dropout_mask) {
					  	
	}	
					
	template <typename Derived>
	void fProp(const MatrixBase<Derived> &data){
		//cerr<<"Data is "<<data<<endl;
		W_x_to_c_node.param->fProp(data,W_x_to_c_node.fProp_matrix);
		W_x_to_f_node.param->fProp(data,W_x_to_f_node.fProp_matrix);
		W_x_to_o_node.param->fProp(data,W_x_to_o_node.fProp_matrix);
		W_x_to_i_node.param->fProp(data,W_x_to_i_node.fProp_matrix);				
			
	}	
	
	template<typename DerivedData, typename DerivedDIn>
	void bProp(const MatrixBase<DerivedData> &data,
				const MatrixBase<DerivedDIn> &o_t_node_bProp_matrix,
				const MatrixBase<DerivedDIn> &i_t_node_bProp_matrix,
				const MatrixBase<DerivedDIn> &f_t_node_bProp_matrix,
				const MatrixBase<DerivedDIn> &tanh_c_prime_t_node_bProp_matrix){
		//cerr<<"input_layer_node.fProp_matrix is "<<input_layer_node.fProp_matrix<<endl;
		//cerr<<"W_x_to_o_node"<<endl;
		int current_minibatch_size = o_t_node_bProp_matrix.cols();
		W_x_to_o_node.param->updateGradient(o_t_node_bProp_matrix,
											data);
		//cerr<<"W_x_to_i_node"<<endl;									
		W_x_to_i_node.param->updateGradient(i_t_node_bProp_matrix,
											data);
		//cerr<<"W_x_to_f_node"<<endl;									
		W_x_to_f_node.param->updateGradient(f_t_node_bProp_matrix,
											data);	
		//cerr<<"W_x_to_c_node"<<endl;									
		W_x_to_c_node.param->updateGradient(tanh_c_prime_t_node_bProp_matrix,
											data);		
											
		d_Err_t_to_n_d_x_t.leftCols(current_minibatch_size) = W_x_to_c_node.bProp_matrix + 
							W_x_to_o_node.bProp_matrix +
							W_x_to_f_node.bProp_matrix +
							W_x_to_i_node.bProp_matrix;	
			
																						
	}
};

} // namespace nplm
#undef EIGEN_NO_AUTOMATIC_RESIZING
