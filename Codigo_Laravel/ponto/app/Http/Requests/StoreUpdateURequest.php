<?php

namespace App\Http\Requests;

use Illuminate\Foundation\Http\FormRequest;

class StoreUpdateURequest extends FormRequest
{
    /**
     * Determine if the user is authorized to make this request.
     *
     * @return bool
     */
    public function authorize()
    {
        return true;
    }

    /**
     * Get the validation rules that apply to the request.
     *
     * @return array
     */
 
    public function rules()
    {
        return [
            //'nome'      => $this->method() == 'POST' ? 'required | min:2 | max: 255' : 'required | min:30',
            'nome'      => 'required|min:2|max: 255',
            'sobrenome' => 'required|min:3|max: 255',
            'status'    => 'required',
            'email'     => 'required|email',
            'password'  =>  $this->method() == 'POST' ? 'required' : '',
        ];
    }
}
