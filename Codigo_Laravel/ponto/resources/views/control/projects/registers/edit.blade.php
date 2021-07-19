{{-- resources/views/admin/dashboard.blade.php --}}

@extends('adminlte::page')

@section('title', "Editar Cadastro de {$user->u}")

{{--@section('content_header')
    <h1>Inserir Usuário</h1>
@stop--}}

@section('content')
    <div class="card">
        <h4 class="card-header"><strong>Editar Dados do Usuário {{  $user->u }}</strong></h4>
        <div class="card-body">
            <form action="{{ route('registers.update', $user->id) }}" method="POST">
                @csrf
                @method('PUT')
                @include('control.projects.registers._partials.form')
            </form>
        </div>
    </div>
@stop