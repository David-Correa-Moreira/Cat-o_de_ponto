<?php

//use Illuminate\Routing\Route;

use Illuminate\Support\Facades\Route;

//todas as rodas devem estar autenticadas para ser acessadas
Route::middleware('auth')->group(function(){


    Route::middleware('verifica_admin')->group(function(){

        Route::delete('registers/users/{id}', 'Admin\UserController@delete')->name('registers.delete');
        Route::put('registers/users/{id}', 'Admin\UserController@update')->name('registers.update');
        Route::get('registers/users/{id}/edit', 'Admin\UserController@edit')->name('registers.edit');
        Route::get('registers/users/create', 'Admin\UserController@create' )->name('registers.create');
        Route::post('registers/users', 'Admin\UserController@store')->name('registers.store');

    });

    Route::get('registers/users', 'Admin\UserController@index' )->name('registers.index');
    Route::get('registers/users/{id}', 'Admin\UserController@show')->name('registers.show');

});


/*
Route::get('/', function () {
    return view('welcome');
});
*/ 

//redireciona pelo url da rota
/*
Route::get('/', function() {
    return redirect('registers/users');
});
*/

//redireciona pelo nome da rota
Route::get('/', function() {
    return redirect()->route('registers.index');
});
//habilita as rotas de autenticação com exceção do registro
Auth::routes(['register' => false]);

Route::get('/home', 'HomeController@index')->name('home');
